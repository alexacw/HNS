#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "SIM868Com.hpp"

static uint8_t data[SERIAL_BUFFERS_SIZE];
static uint32_t readpos = 0;
static uint32_t writepos = 0;
static mutex_t mu;
static void readBufclear(void)
{
    chMtxLock(&mu);
    memset(data, 0, SERIAL_BUFFERS_SIZE);
    readpos = 0;
    writepos = 0;
    chMtxUnlock(&mu);
}

static void readBufInit(void)
{
    chMtxObjectInit(&mu);
    readBufclear();
}
static void readBufWriteFromSD(SerialDriver *sd)
{
    chMtxLock(&mu);
    //read till end of buffer if available, writepos = position for next write
    bool rwasbehind = readpos > writepos;
    uint32_t startWrtiePos = writepos;
    writepos += sdAsynchronousRead(sd, &data[writepos], SERIAL_BUFFERS_SIZE - writepos);
    if (writepos == SERIAL_BUFFERS_SIZE)
    {
        uint32_t writepos = sdAsynchronousRead(sd, data, startWrtiePos);
        if (rwasbehind || readpos < writepos)
            readpos = startWrtiePos; //ditch all if buffer is full
    }
    chMtxUnlock(&mu);
}

size_t readBufGetline(uint8_t line[SERIAL_BUFFERS_SIZE])
{
    chMtxLock(&mu);
    //discard prefix line endings
    while (readpos != writepos)
    {
        if (data[readpos] == '\n' || data[readpos] == '\r')
        {
            readpos++;
            if (readpos == SERIAL_BUFFERS_SIZE)
                readpos = 0;
        }
        else
            break;
    }
    //search for line ending
    size_t i = readpos; // i = position of the first line ending found;

    while (i != writepos && i != SERIAL_BUFFERS_SIZE)
    {
        if (data[i] == '\n' || data[i] == '\r')
        {
            break;
        }
        i = (i + 1) % SERIAL_BUFFERS_SIZE;
    }

    size_t size;
    if (i > readpos)
    {
        size = i - readpos;
        memcpy(line, &data[readpos], size);
    }
    else
    {
        //FIXME:
        memcpy(line, &data[readpos], SERIAL_BUFFERS_SIZE - readpos);
        memcpy(&line[SERIAL_BUFFERS_SIZE - readpos], data, i);
        size = SERIAL_BUFFERS_SIZE + i - readpos;
    }

    chMtxUnlock(&mu);
    return size;
}

static THD_WORKING_AREA(SIM868SerialReadThread_wa, 128);
THD_FUNCTION(SIM868SerialReadThreadFunc, arg)
{
    (void)arg;
    chRegSetThreadName("SIM868SerialRead");

    event_listener_t serial_listener;
    static eventflags_t pending_flags;
    chEvtRegisterMaskWithFlags(chnGetEventSource(&SIM868_SD), &serial_listener,
                               SIM868_SERIAL_EVENT_MASK, SIM868_SERIAL_WK_FLAGS | CHN_INPUT_AVAILABLE); //setup event listening

    while (true)
    {
        chEvtWaitAny(SIM868_SERIAL_EVENT_MASK);                   //wait for selected serial events
        pending_flags = chEvtGetAndClearFlagsI(&serial_listener); //get event flags

        readBufWriteFromSD(&SIM868_SD);
    }
};

void initSIM868Serialhandler()
{
    readBufInit();
    palSetPadMode(GPIOA, GPIOA_USART1_TX, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, GPIOA_USART1_RX, PAL_MODE_INPUT_PULLUP);
    sdStart(&SIM868_SD, &SIM868_SERIAL_CONFIG);
    chThdCreateStatic(SIM868SerialReadThread_wa, sizeof(SIM868SerialReadThread_wa), //Start Judge RX thread
                      NORMALPRIO, SIM868SerialReadThreadFunc, NULL);
};
