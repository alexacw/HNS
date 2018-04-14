#include "UartHandler.hpp"

volatile uint32_t bufend;
uint16_t *buf[100];
UARTConfig myuartconfig;


void initUartHandler(UARTDriver *driver_struct, const uint32_t &baudRate)
{
    uartObjectInit(driver_struct);
    myuartconfig.speed = baudRate;
    myuartconfig.rxend_cb = rxCB;
    uartStart(driver_struct, &myuartconfig);
    uartStartReceive(driver_struct, 1, buf);
};

void rxCB(UARTDriver *driver_struct)
{
    uartStartSend(&UARTD1, 1, buf);
    uartStartReceive(driver_struct, 1, buf);
};

void myUartSend(UARTDriver *driver_struct, size_t size, const void *data)
{
    uartStopReceive(driver_struct);
    uartStartSend(driver_struct, size, data);
    uartStartReceive(driver_struct, 1, buf);
}
