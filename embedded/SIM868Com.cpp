#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"
#include "SIM868Com.hpp"

using namespace sim868Serial;

uint8_t sim868SerialRXBuf[SERIAL_BUFFERS_SIZE];

static THD_WORKING_AREA(SIM868SerialThread_wa, 128);

static THD_FUNCTION(SIM868SerialThread, arg)
{
    (void)arg;
    chRegSetThreadName("SIM868_COM");

    static const eventflags_t serial_wkup_flags =                 //Partially from SD driver
        CHN_INPUT_AVAILABLE | CHN_DISCONNECTED | SD_NOISE_ERROR | //Partially inherited from IO queue driver
        SD_PARITY_ERROR | SD_FRAMING_ERROR | SD_OVERRUN_ERROR |
        SD_BREAK_DETECTED | SD_QUEUE_FULL_ERROR;

    event_listener_t serial_listener;
    static eventflags_t pending_flags;
    static eventflags_t current_flag;
    chEvtRegisterMaskWithFlags(chnGetEventSource(SIM868_SD), &serial_listener,
                               EVENT_MASK(SIM868_SERIAL_EVENT_ID), serial_wkup_flags); //setup event listening

    static bool waitingForInput = false;

    while (!chThdShouldTerminateX())
    {
        chEvtWaitAny(SIM868_SERIAL_EVENT_ID);
        chEvtWaitOneTimeout(SIM868_SERIAL_EVENT_ID, 100);         //wait for selected serial events
        pending_flags = chEvtGetAndClearFlagsI(&serial_listener); //get event flag

        if (pending_flags & CHN_INPUT_AVAILABLE)
        {
            handleInput(sdAsynchronousRead(SIM868_SD, sim868SerialRXBuf, (size_t)SERIAL_BUFFERS_SIZE));
            sdReadTimeout()
        }
        else
            iqResetI(&(SIM868_SD)->iqueue);
        memset((void *)sim868SerialRXBuf, 0, SERIAL_BUFFERS_SIZE); //Flush RX buffer
    }
}

void sim868Serial::handleInput(const size_t &datalength)
{
}

static size_t serialReadLine(uint8_t *const &dataPtr, const sysinterval_t &timeout)
{
    size_t bytesRead = 0;
    if (bytesRead = sdReadTimeout(SIM868_SD, dataPtr, 1, timeout), bytesRead > 0)
    {
        chEvtWaitAny())
        sdAsynchronousRead(SIM868_SD, dataPtr, SERIAL_BUFFERS_SIZE - 1);
    }

    else
};

void sim868Serial::initSIM868Serialhandler()
{

    memset(sim868SerialRXBuf, 0, SERIAL_BUFFERS_SIZE);
    palSetPadMode(GPIOA, GPIOA_USART1_TX, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, GPIOA_USART1_RX, PAL_MODE_INPUT_PULLDOWN);
    sdStart(SIM868_SD, &SIM868_SERIAL_CONFIG);
    chThdCreateStatic(SIM868SerialThread_wa, sizeof(SIM868SerialThread_wa), //Start sim868 serial thread
                      NORMALPRIO + 5, SIM868SerialThread, NULL);
};
