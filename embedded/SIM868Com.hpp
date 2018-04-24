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

static const SerialConfig SIM868_SERIAL_CONFIG = {
    115200,          //Baud Rate
    USART_CR1_UE,    //CR1 Register
    USART_CR2_LINEN, //CR2 Register
    0                //CR3 Register
};

void initSIM868Serialhandler();
