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
    9600,         //Baud Rate
};

void initSIM868Serialhandler(void);

/**
      * @brief store the first line received to the data pointer,
      * then free line from the buffer
      * 
      * @param data 
      * @return uint32_t 
     */
size_t readBufGetline(uint8_t line[SERIAL_BUFFERS_SIZE]);
