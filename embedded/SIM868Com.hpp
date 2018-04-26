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

#define SIM868_SD &SD1

#define SIM868_SERIAL_EVENT_ID 1
#define LEAST_SET_BIT(x) x &(-x)

namespace sim868Serial
{

static const SerialConfig SIM868_SERIAL_CONFIG = {
    9600,            //Baud Rate
    USART_CR1_UE,    //CR1 Register
    USART_CR2_LINEN, //CR2 Register
    0                //CR3 Register
};

static void initSIM868Serialhandler();

static void handleInput(const size_t &datalength);

/**
 * @brief read a line from serial, dataPtr should be allocated with sufficient space
 * 
 * return the number of bytes received as a line (end with CR or LF) in the given time  
 * 
 */
static size_t serialReadLine(uint8_t *const &dataPtr, const sysinterval_t &timeout);
}
