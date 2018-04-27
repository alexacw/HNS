#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "SIM868Com.hpp"

namespace readBuf
{
static uint8_t data[SERIAL_BUFFERS_SIZE];
static uint32_t readpos = 0;
static uint32_t writepos = 0;
static mutex_t mu;
static void clear();
static void init()
{
    chMtxObjectInit(&mu);
    clear();
}
static void clear()
{
    chMtxLock(&mu);
    memset(data, 0, SERIAL_BUFFERS_SIZE);
    chMtxUnlock(&mu);
}
static void writeFromSD(SerialDriver *sd)
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

/**
      * @brief store the first line received to the data pointer,
      * then free line from the buffer
      * 
      * @param data 
      * @return uint32_t 
     */
static size_t getline(uint8_t line[SERIAL_BUFFERS_SIZE])
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
    int i = readpos; // i = position of the first line ending found;

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
        memcpy(line, &data[readpos], SERIAL_BUFFERS_SIZE - readpos);
        memcpy(&line[SERIAL_BUFFERS_SIZE - readpos], data, i);
        size = SERIAL_BUFFERS_SIZE + i - readpos;
    }

    chMtxUnlock(&mu);
}
}

static size_t serialSendNReadTimeout(uint8_t *sendMsg, uint32_t sendMsgLength, uint8_t readDataPtr[SERIAL_BUFFERS_SIZE], const sysinterval_t &timeout){};

static THD_WORKING_AREA(SIM868SerialReadThread_wa, 128);
THD_FUNCTION(SIM868SerialReadThreadFunc, arg)
{
    (void)arg;
    chRegSetThreadName("SIM868SerialRead");

    static const eventflags_t serial_wkup_flags =
        SD_BREAK_DETECTED;

    event_listener_t serial_listener;
    static eventflags_t pending_flags;
    static eventflags_t current_flag;
    chEvtRegisterMaskWithFlags(chnGetEventSource(&SD1), &serial_listener,
                               SIM868_SERIAL_EVENT_MASK, SIM868_SERIAL_WK_FLAGS); //setup event listening

    while (!chThdShouldTerminateX())
    {
        chEvtWaitAny(SIM868_SERIAL_EVENT_MASK);                   //wait for selected serial events
        pending_flags = chEvtGetAndClearFlagsI(&serial_listener); //get event flags
        if (pending_flags & SD_BREAK_DETECTED)
        {
            readBuf::writeFromSD(&SIM868_SD);
        }
    }
};

void initSIM868Serialhandler()
{
    readBuf::init();
    palSetPadMode(GPIOA, GPIOA_USART1_TX, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, GPIOA_USART1_RX, PAL_MODE_INPUT_PULLDOWN);
    sdStart(&SIM868_SD, &SIM868_SERIAL_CONFIG);
    chThdCreateStatic(SIM868SerialReadThread_wa, sizeof(SIM868SerialReadThread_wa), //Start Judge RX thread
                      NORMALPRIO + 5, SIM868SerialReadThreadFunc, NULL);
};
