#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "SIM868Com.hpp"

namespace SIM868Com
{

void readBufclear(void)
{
    chMtxLock(&mu);
    memset(data, 0, SIM868_MSG_BUF_SIZE);
    readpos = 0;
    writepos = 0;
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
    //read till end of buffer if available, writepos = position for next write
    bool rwasbehind = readpos > writepos;
    uint32_t startWrtiePos = writepos;

    //read till end of buffer if available
    if (startWrtiePos == 0)
    {
        //buffer already full when it write to one byte before termination
        writepos += sdAsynchronousRead(sd, &data[startWrtiePos], SERIAL_BUFFERS_SIZE);
    }
    else
    {
        writepos += sdAsynchronousRead(sd, &data[startWrtiePos], (SIM868_MSG_BUF_SIZE - startWrtiePos) > SERIAL_BUFFERS_SIZE ? SERIAL_BUFFERS_SIZE : (SIM868_MSG_BUF_SIZE - startWrtiePos));
    }

    if (rwasbehind && writepos >= readpos)
    {
        //buffer full, stash previous data
        readpos = startWrtiePos;
    }
    if (writepos == SIM868_MSG_BUF_SIZE)
    {
        //wrote to the end, write to the start now
        writepos = sdAsynchronousRead(sd, data, (startWrtiePos - 1) > SERIAL_BUFFERS_SIZE ? SERIAL_BUFFERS_SIZE : (startWrtiePos - 1));
        if (!rwasbehind && writepos >= readpos)
        {
            readpos = startWrtiePos;
        }
    }
    chMtxUnlock(&mu);
}

void readBufPopline()
{
    chMtxLock(&mu);
    int state = 0;
    while (readpos != writepos)
    {
        if (data[readpos] == '\n' || data[readpos] == '\r')
        {
            if (state == 1)
                state = 2;
            readpos++;
            if (readpos == SIM868_MSG_BUF_SIZE)
                readpos = 0;
        }
        else
        {
            if (state == 2)
                break;
            else
            {
                if (state == 0)
                    state = 1;
                readpos++;
                if (readpos == SIM868_MSG_BUF_SIZE)
                    readpos = 0;
            }
        }
    }
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
    uint32_t tempReadpos = readpos;
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
    static eventflags_t pending_flags;
    chEvtRegisterMaskWithFlags(chnGetEventSource(&SIM868_SD), &serial_listener,
                               SIM868_SERIAL_EVENT_MASK, CHN_INPUT_AVAILABLE); //setup event listening

    while (!chThdShouldTerminateX())
    {
        chEvtWaitAny(SIM868_SERIAL_EVENT_MASK);                   //wait for selected serial events
        pending_flags = chEvtGetAndClearFlagsI(&serial_listener); //get event flags

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
    waitCount = sec;
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
    return false;
};

bool initGPS()
{
    SendStr("AT+CGNSPWR=1\r\n");
    return true;
};
}
