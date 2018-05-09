#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "math.h"
#include "SIM868Com.hpp"

namespace SIM868Com
{

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

void readBuffedMsg(SerialDriver *sd)
{
	chMtxLock(&mu);
	//read till end of buffer if available
	writepos += sdAsynchronousRead(sd, &data[writepos], (SIM868_MSG_BUF_SIZE - writepos - 1));
	//this jsut to facilitate c string operation
	data[writepos] = '\0';
	chMtxUnlock(&mu);
}

/**
 * @brief 
 * 
 * @param word the word to find, a standard c string terminated by \0
 * @return int the starting position of the found word in the read queue
 */
int readBufFindWord(const char *word)
{
	chMtxLock(&mu);
	uint32_t tempReadpos = 0;
	while (tempReadpos != writepos)
	{
		int i = 0;
		while (word[i] != '\0' && data[(tempReadpos + i) % SIM868_MSG_BUF_SIZE] == word[i])
		{
			i++;
		}
		if (word[i] == '\0' && i != 0)
		{
			chMtxUnlock(&mu);
			return tempReadpos;
		}
		tempReadpos++;
		if (tempReadpos == SIM868_MSG_BUF_SIZE)
		{
			tempReadpos = 0;
		}
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
	//static eventflags_t pending_flags;
	chEvtRegisterMaskWithFlags(chnGetEventSource(&SIM868_SD), &serial_listener,
							   SIM868_SERIAL_EVENT_MASK, CHN_INPUT_AVAILABLE); //setup event listening

	while (!chThdShouldTerminateX())
	{
		chEvtWaitAny(SIM868_SERIAL_EVENT_MASK); //wait for selected serial events
		//pending_flags = chEvtGetAndClearFlagsI(&serial_listener); //get event flags
		chEvtGetAndClearFlagsI(&serial_listener); //get event flags
		readBuffedMsg(&SIM868_SD);
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
	readThreadPtr = chThdCreateStatic(SIM868SerialReadThread_wa,
									  sizeof(SIM868SerialReadThread_wa),
									  NORMALPRIO,
									  SIM868SerialReadThreadFunc,
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

bool initHTTP()
{
	int trialCount = 0;
	while (trialCount <= 10)
	{
		trialCount++;
		SendStr("AT+SAPBR=0,1\r\n"); //关闭 GPRS 上下文.
		if (waitWordTimeout("OK", 10) < 0)
			break;
		readBufclear();
		chThdSleepMilliseconds(1000 * trialCount);

		SendStr("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
		if (waitWordTimeout("OK", 10) < 0)
			break;
		readBufclear();

		SendStr("AT+SAPBR=3,1,\"APN\",\"cmhk\"\r\n");
		if (waitWordTimeout("OK", 10) < 0)
			break;
		readBufclear();

		SendStr("AT+SAPBR=1,1\r\n"); //激活一个 GPRS 上下文
		if (waitWordTimeout("OK", 10) < 0)
			break;
		readBufclear();

		SendStr("AT+SAPBR=2,1\r\n"); //查询 GPRS 上下文
		if (waitWordTimeout("OK", 10) < 0)
			break;
		readBufclear();

		SendStr("AT+HTTPINIT\r\n"); //init HTTP
		if (waitWordTimeout("OK", 10) < 0)
		{
			readBufclear();
			return true;
		}
	}
	return false;
};

bool termHTTP()
{
	int trialCount = 0;
	while (trialCount <= 5)
	{
		trialCount++;
		SendStr("AT+HTTPTERM\r\n"); //关闭 GPRS 上下文.
		if (waitWordTimeout("OK", 10) < 0)
			break;
		readBufclear();

		SendStr("AT+SAPBR=0,1\r\n"); //init HTTP
		if (waitWordTimeout("OK", 10) < 0)
		{
			readBufclear();
			return true;
		}
	}
	return false;
};

bool initGPS()
{
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
