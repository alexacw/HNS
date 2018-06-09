#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "math.h"
#include "SIM868Com.hpp"
#include "usbcfg.h"
#include "gpsProcess.hpp"

namespace SIM868Com
{
bool monitorSerial = false;

const SerialConfig SIM868_SERIAL_CONFIG = {
	9600u, //Baud Rate
};

thread_t *readThreadPtr = NULL;

THD_WORKING_AREA(SIM868SerialReadThread_wa, 128);

/**
	 * @brief just a very simple buffer, end of string indicated by writepos and always end with a \0 character
	 */
static uint8_t readBuf[SIM868_MSG_BUF_SIZE];
/**
	 * @brief the position of the data array which reading from the serial port should write to, also means its the end of the received message
	 *
	 */
uint32_t writepos = 0;
mutex_t mu;

void readBufclear(void)
{
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
	if (SIM868_MSG_BUF_SIZE - writepos - 1 == 0)
	{
		if (monitorSerial)
			chprintf((BaseSequentialStream *)&SDU1, "\n\nserial buffer full!\n\n");
		chMtxUnlock(&mu);
		iqResetI(&(&SIM868_SD)->iqueue);
		readBufclear();
		return;
	}
	//read till end of buffer if available
	uint32_t dataCount = sdAsynchronousRead(&SIM868_SD, &readBuf[writepos], (SIM868_MSG_BUF_SIZE - writepos - 1));
	//DEBUG only

	if (monitorSerial)
	{
		char temp[64];
		temp[0] = 0;
		if (dataCount < 64 && dataCount > 0)
		{
			memcpy(temp, &readBuf[writepos], dataCount);
			temp[dataCount] = 0;
		}
		chprintf((BaseSequentialStream *)&SDU1, "%s", temp);
	}

	writepos += dataCount;

	//this just to facilitate c string operation
	readBuf[writepos] = 0;
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

unsigned int SendStr(const char *data)
{
	if (monitorSerial)
		chprintf((BaseSequentialStream *)&SDU1, "mcu send to sd:%s", data);
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
	readBufclear();

	SendStr("AT+SAPBR=3,1,\"APN\",\"cmhk\"\r\n");
	if (!waitWordTimeout("OK", 1))
		return false;
	readBufclear();

	SendStr("AT+SAPBR=1,1\r\n"); //activate GPRS
	if (!waitWordStopWordTimeout("OK", "ERROR", 60))
		return false;
	readBufclear();

	SendStr("AT+SAPBR=2,1\r\n"); //query gprs, here will get my ip
	if (waitWordTimeout("OK", 10))
	{
		readBufclear();
		return true;
	}

	return false;
};

bool checkGPRS()
{
	readBufclear();
	SendStr("AT+SAPBR=2,1\r\n"); //query gprs, here will get my ip
	char *p1 = (char *)waitWordTimeout(",", 3);
	if (p1)
	{
		p1 = strtok(p1, ",");
		if (p1)
		{
			return *p1 == '1';
		}
		else
			return false;
	}
	else
		return false;
}

bool deinitGPRS()
{
	readBufclear();
	SendStr("AT+SAPBR=0,1\r\n"); //关闭 GPRS 上下文.
	bool temp = waitWordStopWordTimeout("OK", "ERROR", 60);
	readBufclear();
	return temp;
}

bool HTTP_getFromURL(const char *url)
{
	if (!checkGPRS())
	{
		if (!initGPRS())
			return false;
	}
	readBufclear();

	SendStr("AT+HTTPINIT\r\n"); //init HTTP
	if (!waitWordTimeout("OK", 10))
	{
		SendStr("AT+HTTPTERM\r\n");
		waitWordTimeout("OK", 2);
		return false;
	}

	SendStr("AT+HTTPPARA=\"URL\",\""); //set HTTP parameters
	SendStr(url);
	SendStr("\"\r\n");
	if (!waitWordTimeout("OK", 2))
	{
		return false;
	}

	SendStr("AT+HTTPACTION=0\r\n"); //start get request
	if (waitWordTimeout("200", 20))
	{
		readBufclear();
		SendStr("AT+HTTPTERM\r\n");
		waitWordTimeout("OK", 10);

		return true;
	}
	else
		return false;
};

bool HTTP_postToURL(const char *url)
{
	int trialCount = 0;
	while (trialCount <= 10)
	{
		trialCount++;
		readBufclear();
		SendStr("AT+HTTPINIT\r\n"); //init HTTP
		if (!waitWordTimeout("OK", 10))
		{
			SendStr("AT+HTTPTERM\r\n");
			waitWordTimeout("OK", 10);
			continue;
		}

		SendStr("AT+HTTPPARA=\"URL\",\""); //set HTTP parameters
		SendStr(url);
		SendStr("\"\r\n");
		if (!waitWordTimeout("OK", 10))
		{
			continue;
		}

		SendStr("AT+HTTPACTION=1\r\n"); //start post request
		if (waitWordTimeout("200", 20))
		{
			readBufclear();
			SendStr("AT+HTTPTERM\r\n");
			waitWordTimeout("OK", 10);

			return true;
		}
	}
	return false;
};

bool termGPRS()
{
	int trialCount = 0;
	while (trialCount <= 5)
	{
		trialCount++;
		SendStr("AT+SAPBR=0,1\r\n"); //terminate IP
		if (waitWordTimeout("OK", 30))
		{
			readBufclear();
			return true;
		}
	}
	return false;
};

bool emailParent(const char*)
{

};

bool getGPS()
{
	double estLat, estLong;
	readBufclear();

	SendStr("AT+CGNSPWR=1\r\n");
	if (!waitWordTimeout("OK", 1))
		return false;
	readBufclear();

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
			if (*tempCharPtr != '1')
			{
				//GPS not fixed
				//wait?
				return false;
			}
			receivedTimedate = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "got time (yyyyMMddhhmmss.sss): %s\n", receivedTimedate);
			receivedLattitue = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "got lattitude: %s\n", receivedLattitue);
			receivedLongitude = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "got longitude: %s\n", receivedLongitude);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "got altitude:%s\n", tempCharPtr);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "got Speed Over Ground (Km/h): %s\n", tempCharPtr);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "got Course Over Ground (Degrees): %s\n", tempCharPtr);
			tempCharPtr = (char *)strtok(NULL, ","); //fix mode
			tempCharPtr = (char *)strtok(NULL, ","); //reserved, tempCharPtr should be /0
			receivedHDOP = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "got HDOP: %s\n", receivedHDOP);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "got PDOP: %s\n", tempCharPtr);
			tempCharPtr = (char *)strtok(NULL, ",");
			chprintf((BaseSequentialStream *)&SDU1, "got VDOP: %s\n", tempCharPtr);
			if (GeoPost::update(receivedLattitue, receivedLongitude, receivedHDOP, receivedTimedate))
			{
				GeoPost::getEstimate(estLat, estLong);
				chprintf((BaseSequentialStream *)&SDU1, "filtered estimate lattidue: %d.%d, longitude: %d.%d\n",
						 (int)estLat,
						 ((int)(estLat * 100000.0)) % 100000,
						 (int)estLong,
						 ((int)(estLong * 100000.0)) % 100000);
				chThdSleepMilliseconds(100);
				return true;
			}
		}
	}
}
} // namespace SIM868Com