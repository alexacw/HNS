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

static uint8_t sdrxbuf[SERIAL_BUFFERS_SIZE];
SerialConfig sim868serialconfig;

void initSIM868Serialhandler()
{
    palSetPadMode(GPIOA, GPIOA_USART1_TX, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, GPIOA_USART1_RX, PAL_MODE_INPUT_PULLDOWN);
    sdStart(SIM868_SD, &SIM868_SERIAL_CONFIG);
};

static THD_WORKING_AREA(JudgeThread_wa, 1024);

static THD_FUNCTION(JudgeThread, arg)
{

    (void)arg;
    chRegSetThreadName("SIM868 COM");

    memset((void *)sdrxbuf, 0, SERIAL_BUFFERS_SIZE);

    static const eventflags_t serial_wkup_flags =                 //Partially from SD driver
        CHN_INPUT_AVAILABLE | CHN_DISCONNECTED | SD_NOISE_ERROR | //Partially inherited from IO queue driver
        SD_PARITY_ERROR | SD_FRAMING_ERROR | SD_OVERRUN_ERROR |
        SD_BREAK_DETECTED;

    event_listener_t serial_listener;
    static eventflags_t pending_flags;
    static eventflags_t current_flag;
    chEvtRegisterMaskWithFlags(chnGetEventSource(SIM868_SD), &serial_listener,
                               EVENT_MASK(SIM868_SERIAL_EVENT_ID), serial_wkup_flags); //setup event listening

    while (!chThdShouldTerminateX())
    {

        chEvtWaitAny(SIM868_SERIAL_EVENT_ID);                     //wait for selected serial events
        pending_flags = chEvtGetAndClearFlagsI(&serial_listener); //get event flag

        if (pending_flags & CHN_INPUT_AVAILABLE)
        {
            handleInput(sdAsynchronousRead(SIM868_SD, sdrxbuf, (size_t)SERIAL_BUFFERS_SIZE));
        }

        chIQResetI(&(sdp)->iqueue);
        memset((void *)sdrxbuf, 0, JUDGE_BUFFER_SIZE); //Flush RX buffer
    }
}
