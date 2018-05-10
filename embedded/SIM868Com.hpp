
#ifndef _SIM868COM_
#define _SIM868COM_
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

namespace SIM868Com
{

extern thread_t *readThreadPtr;
extern THD_WORKING_AREA(SIM868SerialReadThread_wa, 128);

/**
 * @brief just a very simple buffer, end of string indicated by writepos and always end with a \0 character
 */
extern uint8_t data[SIM868_MSG_BUF_SIZE];
/**
 * @brief the position of the data array which reading from the serial port should write to, also means its the end of the received message
 * 
 */
extern uint32_t writepos;
extern mutex_t mu;

void initSerial();
void startSerialRead();
void stopSerialRead();

void readBufInit();
void readBufclear();
void readBuffedMsg();
bool readBufWaitLine(int sec); //wait until \n or \r is received
int readBufFindWord(const char *word);
int waitWordTimeout(const char *word, int sec);
int waitWordStopWordTimeout(const char *word, const char *termword, int sec);

unsigned int SendStr(const char *);
unsigned int SendChar(const char);

bool initIP();
bool termIP();
bool HTTP_getFromURL(const char *url);
bool initGPS();
} // namespace SIM868Com

float str2f(const char *str_num);

#endif
