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
#include "stm32f1xx.h"

#define SIM868_SD SD1

#define SIM868_SERIAL_EVENT_ID 1
#define SIM868_SERIAL_EVENT_MASK EVENT_MASK(SIM868_SERIAL_EVENT_ID)
#define LEAST_SET_BIT(x) x &(-x)
#define SIM868_SERIAL_WK_FLAGS SD_BREAK_DETECTED

static const SerialConfig SIM868_SERIAL_CONFIG = {
    9600,            //Baud Rate
    USART_CR1_UE,    //CR1 Register
    USART_CR2_LINEN, //CR2 Register
    0                //CR3 Register
};

static void initSIM868Serialhandler();

/**
 * @brief read a line from serial, dataPtr should be allocated with sufficient space
 * 
 * return the number of bytes received as a line (end with CR or LF) in the given time  
 * 
 */
static size_t serialSendNReadTimeout(uint8_t *sendMsg, uint32_t sendMsgLength, uint8_t readDataPtr[SERIAL_BUFFERS_SIZE], const sysinterval_t &timeout);
