#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

void initUartHandler(UARTDriver *driver_struct, const uint32_t &baudRate);

void myUartSend(UARTDriver *driver_struct, size_t size, const void *data);


void rxCB(UARTDriver *driver_struct);
