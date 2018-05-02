#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "SIM868Com.hpp"

namespace SIM868Com
{

static void readBufclear(void)
{
    chMtxLock(&mu);
    memset(data, 0, SIM868_MSG_BUF_SIZE);
    readpos = 0;
    writepos = 0;
    chMtxUnlock(&mu);
}

static void readBufInit(void)
{
    chMtxObjectInit(&mu);
    readBufclear();
}
static void readBuffedMsg(SerialDriver *sd)
{
    chMtxLock(&mu);
    //read till end of buffer if available, writepos = position for next write
    bool rwasbehind = readpos > writepos;
    uint32_t startWrtiePos = writepos;
    writepos += sdAsynchronousRead(sd, &data[writepos], SIM868_MSG_BUF_SIZE - writepos);
    if (writepos == SIM868_MSG_BUF_SIZE)
    {
        uint32_t writepos = sdAsynchronousRead(sd, data, startWrtiePos);
        if (rwasbehind || readpos < writepos)
            readpos = startWrtiePos; //ditch all if buffer is full
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

int readBufFindWord(char *word, int size)
{
    chMtxLock(&mu);
    uint32_t tempReadpos = readpos;
    while (tempReadpos != writepos)
    {
        int i = 0;
        for (; i < size && data[tempReadpos + i] == word[i]; i++)
            ;
        if (i == size)
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
    return -1;
    chMtxUnlock(&mu);
}

static THD_WORKING_AREA(SIM868SerialReadThread_wa, 128);
THD_FUNCTION(SIM868SerialReadThreadFunc, arg)
{
    (void)arg;
    chRegSetThreadName("SIM868SerialRead");

    event_listener_t serial_listener;
    static eventflags_t pending_flags;
    chEvtRegisterMaskWithFlags(chnGetEventSource(&SIM868_SD), &serial_listener,
                               SIM868_SERIAL_EVENT_MASK, CHN_INPUT_AVAILABLE); //setup event listening

    while (chThdShouldTerminateX())
    {
        chEvtWaitAny(SIM868_SERIAL_EVENT_MASK);                   //wait for selected serial events
        pending_flags = chEvtGetAndClearFlagsI(&serial_listener); //get event flags

        readBuffedMsg(&SIM868_SD);
    }
};

void initSIM868Serialhandler()
{
    readBufInit();
    palSetPadMode(GPIOA, GPIOA_USART1_TX, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    palSetPadMode(GPIOA, GPIOA_USART1_RX, PAL_MODE_INPUT_PULLUP);
    sdStart(&SIM868_SD, &SIM868_SERIAL_CONFIG);
    chThdCreateStatic(SIM868SerialReadThread_wa, sizeof(SIM868SerialReadThread_wa), //Start Judge RX thread
                      NORMALPRIO, SIM868SerialReadThreadFunc, NULL);
};
}