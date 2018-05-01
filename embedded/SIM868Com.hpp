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
#define SIM868_MSG_BUF_SIZE 256

static const SerialConfig SIM868_SERIAL_CONFIG = {
    9600, //Baud Rate
};

static uint8_t data[SIM868_MSG_BUF_SIZE];
static uint32_t readpos = 0;
static uint32_t writepos = 0;

void initSIM868Serialhandler(void);

/**
      * @brief store the first line received to the data pointer,
      * then free line from the buffer
      * 
      * @param data 
      * @return uint32_t 
     */
void readBufPopline();

int readBufFindWord(char *word, int size);

bool waitForOK(sysinterval_t timeout);

static mutex_t mu;
