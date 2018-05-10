#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "math.h"
#include "SIM868Com.hpp"

namespace SIM868Com
{

const SerialConfig SIM868_SERIAL_CONFIG = {
	9600, //Baud Rate
};

thread_t *readThreadPtr = NULL;
THD_WORKING_AREA(SIM868SerialReadThread_wa, 128);

/**
	 * @brief just a very simple buffer, end of string indicated by writepos and always end with a \0 character
	 */
uint8_t data[SIM868_MSG_BUF_SIZE];
/**
	 * @brief the position of the data array which reading from the serial port should write to, also means its the end of the received message
	 *
	 */
uint32_t writepos = 0;
mutex_t mu;

void readBufclear(void)
{
	chMtxLock(&mu);
	memset(data, 0, SIM868_MSG_BUF_SIZE);
	writepos = 0;
	data[0] = '\0';
	chMtxUnlock(&mu);
}

void readBufInit(void)
{
	chMtxLock(&mu);
	chMtxObjectInit(&mu);
	readBufclear();
	chMtxUnlock(&mu);
}

void readBuffedMsg()
{
	chMtxLock(&mu);
	//read till end of buffer if available
	writepos += sdAsynchronousRead(&SIM868_SD, &data[writepos], (SIM868_MSG_BUF_SIZE - writepos - 1));
	//this jsut to facilitate c string operation
	data[writepos] = '\0';
	chMtxUnlock(&mu);
}

bool readBufWaitLine(int sec)
{
	static int waitCount;
	waitCount = sec * 10;
	while (waitCount >= 0)
	{
		chMtxLock(&mu);
		static uint8_t *dataPtr = data;
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
int readBufFindWord(const char *word)
{
	chMtxLock(&mu);
	uint32_t wordStartPos = 0;
	while (wordStartPos != writepos)
	{
		int i = 0;
		while (word[i] != '\0' && data[(wordStartPos + i)] == word[i])
		{
			i++;
		}
		if (word[i] == '\0' && i != 0)
		{
			chMtxUnlock(&mu);
			return wordStartPos;
		}
		wordStartPos++;
	}
	chMtxUnlock(&mu);
	return -1;
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
		if (pending_flags & !CHN_INPUT_AVAILABLE)
		{
			iqResetI(&(&SIM868_SD)->iqueue);
		}
		else if (pending_flags & ~CHN_INPUT_AVAILABLE)
			readBuffedMsg();
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

int waitWordTimeout(const char *word, int sec)
{
	static int waitCount;
	static int wordPos;
	waitCount = sec * 10;
	while (waitCount >= 0)
	{
		if ((wordPos = SIM868Com::readBufFindWord(word)) >= 0)
		{
			return wordPos;
		}
		chThdSleepMilliseconds(100);
		waitCount--;
	}
	return -1;
};
int waitWordStopWordTimeout(const char *word, const char *termword, int sec)
{
	static int waitCount;
	static int wordPos;
	waitCount = sec * 10;
	while (waitCount >= 0)
	{
		if ((wordPos = SIM868Com::readBufFindWord(word)) >= 0)
		{
			return wordPos;
		}
		else if (SIM868Com::readBufFindWord(termword) >= 0)
		{
			return -1;
		}
		chThdSleepMilliseconds(100);
		waitCount--;
	}
	return -1;
};

unsigned int SendStr(const char *data)
{
	if (data != NULL)
	{
		static uint32_t size;
		for (size = 0; data[size] != '\0'; size++)
			;
		return sdWriteI(&SIM868_SD, (uint8_t *)data, size);
	}
	else
		return 0;
};

unsigned int SendChar(const char letter)
{

	return sdWriteI(&SIM868_SD, (const uint8_t *)&letter, 1);
};

bool initIP()
{
	int trialCount = 0;
	while (trialCount <= 10)
	{
		trialCount++;
		readBufclear();
		SendStr("AT+SAPBR=0,1\r\n"); //关闭 GPRS 上下文.
		if (waitWordTimeout("OK", 10) < 0)
			continue;
		readBufclear();
		chThdSleepMilliseconds(1000 * trialCount);

		SendStr("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
		if (waitWordTimeout("OK", 10) < 0)
			continue;
		readBufclear();

		SendStr("AT+SAPBR=3,1,\"APN\",\"cmhk\"\r\n");
		if (waitWordTimeout("OK", 10) < 0)
			continue;
		readBufclear();

		SendStr("AT+SAPBR=1,1\r\n"); //激活一个 GPRS 上下文
		if (waitWordTimeout("OK", 10) < 0)
			continue;
		readBufclear();

		SendStr("AT+SAPBR=2,1\r\n"); //查询 GPRS 上下文
		if (waitWordTimeout("OK", 10) >= 0)
		{
			readBufclear();
			return true;
		}
	}
	return false;
};

bool HTTP_getFromURL(const char *url)
{
	int trialCount = 0;
	while (trialCount <= 10)
	{
		trialCount++;
		readBufclear();
		SendStr("AT+HTTPINIT\r\n"); //init HTTP
		if (waitWordTimeout("OK", 10) < 0)
		{
			SendStr("AT+HTTPTERM\r\n");
			waitWordTimeout("OK", 10);
			continue;
		}

		SendStr("AT+HTTPPARA=\"URL\",\""); //set HTTP parameters
		SendStr(url);
		SendStr("\"\r\n");
		if (waitWordTimeout("OK", 10) < 0)
		{
			continue;
		}

		SendStr("AT+HTTPACTION=0"); //start get request
		if (waitWordTimeout("200", 20) >= 0)
		{
			readBufclear();
			SendStr("AT+HTTPTERM\r\n");
			waitWordTimeout("OK", 10);

			return true;
		}
	}
	return false;
};

bool termIP()
{
	int trialCount = 0;
	while (trialCount <= 5)
	{
		trialCount++;
		SendStr("AT+SAPBR=0,1\r\n"); //terminate IP
		if (waitWordTimeout("OK", 10) >= 0)
		{
			readBufclear();
			return true;
		}
	}
	return false;
};

bool initGPS()
{
	//TODO:
	int trialCount = 10;
	while (trialCount > 0)
	{
		SendStr("AT+CGNSPWR=1\r\n");
		if (waitWordTimeout("OK", 1) < 0)
			break;
	}

	return true;
};
} // namespace SIM868Com

float str2f(const char *str_num)
{
	float temp = 0;
	int dpCount = -1;
	while (*str_num != '\0')
	{
		if (*str_num >= '0' && *str_num <= '9')
		{
			temp *= 10.0;
			temp += *str_num - '0';
			if (dpCount >= 0)
			{
				dpCount++;
			}
		}
		else if (*str_num == '.')
		{
			if (dpCount < 0)
			{
				dpCount = 0;
			}
			else
				return NAN; //return NaN if the input is not a valid number, check by (f==f), false for this
		}
		else
			return NAN; //not all digits and .
		str_num++;
	}
	for (int i = 0; i < dpCount; i++)
	{
		temp /= 10.0;
	}
	return temp;
}
