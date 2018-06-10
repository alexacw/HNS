#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "math.h"
#include "SIM868Com.hpp"
#include "usbcfg.h"
#include "GeoPost.hpp"
#include "flash.hpp"

namespace SIM868Com
{
bool monitorSerial = false;
bool receivedCall = false;
bool receivedNewSMS = false;
bool gprsAva = false;
bool outBound = false;
bool aggressive = false;
bool calmDownAlert = false;

const SerialConfig SIM868_SERIAL_CONFIG = {
	9600u, //Baud Rate
};

thread_t *readThreadPtr = NULL;

THD_WORKING_AREA(SIM868SerialReadThread_wa, 128);

/**
	 * @brief just a very simple buffer, end of string indicated by writepos and always end with a \0 character
	 */
static uint8_t readBuf[SIM868_MSG_BUF_SIZE] = {0};
/**
	 * @brief the position of the data array which reading from the serial port should write to, also means its the end of the received message
	 *
	 */
uint32_t writepos = 0;
mutex_t mu;

void readBufclear(void)
{
	chThdSleepMilliseconds(300);
	findKeywords();
	chMtxLock(&mu);
	writepos = 0;
	readBuf[0] = 0;
	chMtxUnlock(&mu);
}

void readBufInit(void)
{
	chMtxObjectInit(&mu);
	readBufclear();
}

void readBuffedMsg()
{
	chMtxLock(&mu);
	//buffer filled up, copy last line of message, determined by \n \r, to the front and continue
	if (writepos >= SIM868_MSG_BUF_SIZE - 1)
	{
		int i;
		for (i = SIM868_MSG_BUF_SIZE - 2; i >= 0; i--)
		{
			if (readBuf[i] == '\n' || readBuf[i] == '\r' || readBuf[i] == '\0')
			{
				memcpy(&readBuf[0], &readBuf[i + 1], (SIM868_MSG_BUF_SIZE - 1) - (i + 1));
				readBuf[(SIM868_MSG_BUF_SIZE - 2) - (i + 1) + 1] = 0;
				writepos = (SIM868_MSG_BUF_SIZE - 2) - (i + 1) + 1;

				//read remaining
				//read till end of buffer if available
				int tempdatal = sdAsynchronousRead(&SIM868_SD, &readBuf[writepos], (SIM868_MSG_BUF_SIZE - writepos - 1));

				readBuf[writepos + tempdatal] = 0;
				if (monitorSerial)
				{
					chprintf((BaseSequentialStream *)&SDU1, (char *)&readBuf[writepos]);
				}

				writepos += tempdatal;

				if (writepos >= SIM868_MSG_BUF_SIZE - 1)
				{
					chprintf((BaseSequentialStream *)&SDU1, "FATAL: readBuf run out of memory, cannnot store a full line");
				}
				break;
			}
		}
	}
	else
	{
		//read till end of buffer if available
		int tempdatal = sdAsynchronousRead(&SIM868_SD, &readBuf[writepos], (SIM868_MSG_BUF_SIZE - writepos - 1));
		readBuf[writepos + tempdatal] = 0;

		if (monitorSerial)
		{
			chprintf((BaseSequentialStream *)&SDU1, (char *)&readBuf[writepos]);
		}
		writepos += tempdatal;
	}

	//this just to facilitate c string operation

	chMtxUnlock(&mu);
}

bool readBufWaitLine(int sec)
{
	static int waitCount;
	waitCount = sec * 10;
	while (waitCount >= 0)
	{
		chMtxLock(&mu);
		static uint8_t *dataPtr = readBuf;
		while (*dataPtr != '\0')
		{
			if (*dataPtr == '\r' || *dataPtr == '\n')
			{
				chMtxUnlock(&mu);
				return true;
			}
			dataPtr++;
		}
		chMtxUnlock(&mu);
		chThdSleepMilliseconds(100);
		waitCount--;
	}
	return false;
};

/**
	 * @brief
	 *
	 * @param word the word to find, a standard c string terminated by \0
	 * @return int the starting position of the found word in the read queue
	 */
const char *readBufFindWord(const char *word)
{
	chMtxLock(&mu);
	const char *result = strstr((char *)&SIM868Com::readBuf[0], word);
	chMtxUnlock(&mu);
	return result;
}
//Serial listenser
static THD_FUNCTION(SIM868SerialReadThreadFunc, arg)
{
	(void)arg;
	chRegSetThreadName("SIM868SerialRead");

	static event_listener_t serial_listener;
	static const eventflags_t tolis = CHN_INPUT_AVAILABLE | CHN_DISCONNECTED | SD_NOISE_ERROR | //Partially inherited from IO queue driver
									  SD_PARITY_ERROR | SD_FRAMING_ERROR | SD_OVERRUN_ERROR | SD_BREAK_DETECTED;
	chEvtRegisterMaskWithFlags(chnGetEventSource(&SIM868_SD), &serial_listener,
							   SIM868_SERIAL_EVENT_MASK, tolis); //setup event listening

	while (!chThdShouldTerminateX())
	{
		chEvtWaitAny(SIM868_SERIAL_EVENT_MASK); //wait for selected serial events
		static eventflags_t pending_flags;
		pending_flags = chEvtGetAndClearFlagsI(&serial_listener); //get event flags
		if (pending_flags & CHN_INPUT_AVAILABLE)
			readBuffedMsg();
		else
		{
			iqResetI(&(&SIM868_SD)->iqueue);
		}
	}
};

void initSerial()
{
	readBufInit();
	palSetPadMode(GPIOA, GPIOA_USART1_TX, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
	palSetPadMode(GPIOA, GPIOA_USART1_RX, PAL_MODE_INPUT_PULLUP);
	sdStart(&SIM868_SD, &SIM868_SERIAL_CONFIG);
};

void startSerialRead()
{
	readThreadPtr = chThdCreateStatic(SIM868SerialReadThread_wa, sizeof(SIM868SerialReadThread_wa),
									  NORMALPRIO, SIM868SerialReadThreadFunc,
									  NULL);
};

void stopSerialRead()
{
	chMtxLock(&mu);
	chThdTerminate(readThreadPtr);
	chMtxUnlock(&mu);
};

void initModulePara()
{
	// do
	// {
	// 	readBufclear();
	// 	SendStr("ATE0\r\n"); //set no tx loopback to save buffer size
	// } while (!waitWordTimeout("OK", 1));

	do
	{
		readBufclear();
		SendStr("AT+CSCA=\"+85292040031\"\r\n"); //set center number
	} while (!waitWordTimeout("OK", 1));
	do
	{
		readBufclear();
		SendStr("AT+CMGF=1\r\n"); //set sms to text mode
	} while (!waitWordTimeout("OK", 1));
};

const char *waitWordTimeout(const char *word, int sec)
{
	static int waitCount;
	static const char *wordPos;
	waitCount = sec * 10;
	while (waitCount >= 0)
	{
		if ((wordPos = SIM868Com::readBufFindWord(word)))
		{
			return wordPos;
		}
		chThdSleepMilliseconds(100);
		waitCount--;
	}
	return NULL;
};

const char *waitWordStopWordTimeout(const char *word, const char *termword, int sec)
{
	static int waitCount;
	static const char *wordPos;
	waitCount = sec * 10;
	while (waitCount >= 0)
	{
		if ((wordPos = SIM868Com::readBufFindWord(word)))
		{
			return wordPos;
		}
		else if (SIM868Com::readBufFindWord(termword))
		{
			return NULL;
		}
		chThdSleepMilliseconds(100);
		waitCount--;
	}
	return NULL;
};

//basically just a overlay, to facillate debug
unsigned int SendStr(const char *data)
{
	readBufclear();
	// if (monitorSerial)
	// 	chprintf((BaseSequentialStream *)&SDU1, "TX: %s", data);
	if (data != NULL)
	{
		static uint32_t size;
		for (size = 0; data[size] != '\0'; size++)
			;
		return sdWrite(&SIM868_SD, (uint8_t *)data, size);
	}
	else
		return 0;
};

unsigned int SendChar(const char letter)
{

	return sdWriteI(&SIM868_SD, (const uint8_t *)&letter, 1);
};

bool initGPRS()
{
	SendStr("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
	if (!waitWordTimeout("OK", 1))
		return false;

	SendStr("AT+SAPBR=3,1,\"APN\",\"cmhk\"\r\n");
	if (!waitWordTimeout("OK", 1))
		return false;

	SendStr("AT+SAPBR=1,1\r\n"); //activate GPRS
	if (!waitWordStopWordTimeout("OK", "ERROR", 100))
		return false;

	return checkGPRS();
};

bool checkGPRS()
{
	SendStr("AT+SAPBR=2,1\r\n"); //query gprs, here will get my ip
	char *p1 = (char *)waitWordTimeout(",", 3);
	if (p1)
	{
		p1 = strtok(p1, ",");
		if (p1)
		{
			chprintf((BaseSequentialStream *)&SDU1, "checkGPRS: %s\n", (*p1 == '1') ? "available" : "dead");
			if (*p1 == '1')
			{
				gprsAva = true;
				return true;
			}
		}
	}
	gprsAva = false;
	return false;
}

bool deinitGPRS()
{
	readBufclear();
	SendStr("AT+SAPBR=0,1\r\n"); //关闭 GPRS 上下文.
	bool temp = waitWordStopWordTimeout("OK", "ERROR", 100);

	return temp;
}

bool HTTP_getFromURL(const char *url)
{
	if (!checkGPRS())
	{
		if (!initGPRS())
			return false;
	}

	SendStr("AT+HTTPINIT\r\n"); //init HTTP
	waitWordStopWordTimeout("OK", "ERROR", 2);

	SendStr("AT+HTTPPARA=\"URL\",\""); //set HTTP parameters
	SendStr(url);
	SendStr("\"\r\n");
	if (!waitWordTimeout("OK", 2))
	{
		return false;
	}

	SendStr("AT+HTTPACTION=0\r\n"); //start get request
	if (!waitWordTimeout("200", 30))
	{
		return true;
	}
};

bool HTTP_getLocStatus()
{
	SendStr("AT+HTTPREAD\r\n"); //start get request
	if (waitWordTimeout("OK", 5))
	{
		if (waitWordTimeout("OUT", 1))
		{
			outBound = true;
			chprintf((BaseSequentialStream *)&SDU1, "device out of bound\n");
		}
		else
		{
			outBound = false;
		}
		return true;
	}
}

bool turnoffGPS()
{
	SendStr("AT+CGNSPWR=0\r\n");

	if (!waitWordTimeout("OK", 1))
		return false;
	return true;
};

bool updateGPS()
{
	double estLat, estLong;

	SendStr("AT+CGNSPWR=1\r\n");
	if (!waitWordTimeout("OK", 1))
		return false;

	SendStr("AT+CGNSINF\r\n");
	char *p1 = (char *)waitWordTimeout("CGNSINF:", 3);
	if (p1) //寻找开始符
	{
		char *tempCharPtr;
		if ((tempCharPtr = (char *)waitWordTimeout("OK", 3))) //module will send "OK" after the GPS info, so wait for this
		{
			char *receivedLattitue = NULL;
			char *receivedLongitude = NULL;
			char *receivedTimedate = NULL;
			char *receivedHDOP = NULL;

			*tempCharPtr = 0;			   //set the char position of "OK" to be eol
			tempCharPtr = strtok(p1, ":"); //skip the "CGNSINF:" part
			tempCharPtr = strtok(NULL, ",");
			if (!strstr(tempCharPtr, "1"))
			{
				do
				{

					SendStr("AT+CGNSPWR=1\r\n");
				} while (!waitWordTimeout("OK", 1));
			}
			tempCharPtr = (char *)strtok(NULL, ",");
			bool fixed = (*tempCharPtr == '1');
			chprintf((BaseSequentialStream *)&SDU1, "\n\nGot GPS info:\n");
			receivedTimedate = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "timestamp: %s\n", receivedTimedate);
			if (!fixed)
			{
				chprintf((BaseSequentialStream *)&SDU1, "!!! gps not fix\r\n");
				return false;
			}
			receivedLattitue = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "\tgot lattitude: %s\n", receivedLattitue);
			receivedLongitude = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "\tgot longitude: %s\n", receivedLongitude);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "\tgot altitude:%s\n", tempCharPtr);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "\tgot Speed Over Ground (Km/h): %s\n", tempCharPtr);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "\tgot Course Over Ground (Degrees): %s\n", tempCharPtr);
			tempCharPtr = (char *)strtok(NULL, ","); //fix mode
			tempCharPtr = (char *)strtok(NULL, ","); //reserved, tempCharPtr should be /0
			receivedHDOP = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "\tgot HDOP: %s\n", receivedHDOP);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "\tgot PDOP: %s\n", tempCharPtr);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "\tgot VDOP: %s\n", tempCharPtr);
			chprintf((BaseSequentialStream *)&SDU1, "	updating Kalman filter... %s\n", tempCharPtr);
			if (GeoPost::update(receivedLattitue, receivedLongitude, receivedHDOP, receivedTimedate))
			{
				GeoPost::getEstimate(estLat, estLong);
				chprintf((BaseSequentialStream *)&SDU1, "	filtered estimate lattidue: %d.%05d, longitude: %d.%05d\n",
						 (int)estLat,
						 ((int)(estLat * 100000.0)) % 100000,
						 (int)estLong,
						 ((int)(estLong * 100000.0)) % 100000);
				return true;
			}
		}
	}
};

bool updateGSMLoc(char *timedate, double &lat, double &lng)
{
	SendStr("AT+CIPGSMLOC=1,1\r\n");
	if (waitWordTimeout("OK", 10))
	{
		const char *tempCharPtr = strtok((char *)&readBuf[0], ":");
		tempCharPtr = strtok(NULL, ",");
		if (tempCharPtr = strtok(NULL, ","))
		{
			chprintf((BaseSequentialStream *)&SDU1, "Got gsm longitude: %s\n", tempCharPtr);
			lng = atof(tempCharPtr);
		}
		if (tempCharPtr = strtok(NULL, ","))
		{
			chprintf((BaseSequentialStream *)&SDU1, "Got gsm lattitude: %s\n", tempCharPtr);
			lat = atof(tempCharPtr);
		}
		if (tempCharPtr = strtok(NULL, ","))
		{
			chprintf((BaseSequentialStream *)&SDU1, "Got gsm date: %s\n", tempCharPtr);
		}
		if (tempCharPtr = strtok(NULL, "\n"))
		{
			chprintf((BaseSequentialStream *)&SDU1, "Got gsm time: %s\n", tempCharPtr);
		}
	}
	else
		return false;
}

bool sendSMS(const char *receiverNumber, const char *message)
{

	SIM868Com::initModulePara();

	SendStr("AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n"); //set sms to store all to sim card
	if (!waitWordTimeout("OK", 1))
		return false;

	SendStr("AT+CSCA=\"+85292040031\"\r\n"); //set center number (actuall will be stored in the module, can do it just once via usb)
	if (!waitWordTimeout("OK", 1))
		return false;

	SendStr("AT+CMGS=\"");
	SendStr(receiverNumber);
	SendStr("\"\r\n"); //set receiver number
	if (!waitWordTimeout(">", 2))
		return false;

	SendStr(message); //set message body
	SendChar(0x1a);   //send end of message
	if (!waitWordTimeout("+CMGS:", 2))
		return false;
	return true;
};

bool unreadSMSFindSender(const char *senderNumber)
{

	SendStr("AT+CMGL=\"REC UNREAD\",0\r\n");
	return (waitWordTimeout(senderNumber, 10));
};

//for finding keywords like receiving a phone call or sms
void findKeywords()
{
	//receiving a call
	if (readBufFindWord("+CCWA"))
	{
		receivedCall = true;
	}
	//receiving a message
	if (readBufFindWord("+CMT"))
	{
		receivedNewSMS = true;
	}
};

void reportToSMS(char *updatetime, const double &lat, const double &lng)
{
	char tempMsg[128] = {0};
	sprintf(tempMsg,
			"lastSeen %s \n lattidue: %d.%05d\n longitude: %d.%05d",
			updatetime,
			(int)lat,
			((int)(lat * 100000.0)) % 100000,
			(int)lng,
			((int)(lng * 100000.0)) % 100000);
	sendSMS(flashStorage::content.parentTel, tempMreportToSMSsg);
}

bool reportToServer(const double &lat, const double &lng)
{
	char tempMsg[256] = {0};
	sprintf(tempMsg,
			"http://128.199.83.132/GPSTracker/input.php?user_id=%ld&lat=%d.%05d&lng=%d.%05d",
			flashStorage::content.deviceID,
			(int)lat,
			((int)(lat * 100000.0)) % 100000,
			(int)lng,
			((int)(lng * 100000.0)) % 100000);
	return HTTP_getFromURL(tempMsg);
}

} // namespace SIM868Com