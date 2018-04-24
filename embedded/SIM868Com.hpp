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

static const SerialConfig SERIAL_JUDGE_CONFIG = {
  115200,               //Baud Rate
  USART_CR1_UE,         //CR1 Register
  USART_CR2_LINEN,      //CR2 Register
  0                     //CR3 Register
};

void initSerialHandler();

void myUartSend(UARTDriver *driver_struct, size_t size, const void *data);

void rxCB(UARTDriver *driver_struct);
