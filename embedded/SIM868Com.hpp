
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
#define SIM868_MSG_BUF_SIZE 128

namespace SIM868Com
{
void initSerial();
void startSerialRead();
void stopSerialRead();

void readBufInit();
void readBufclear();
void readBuffedMsg();
bool readBufWaitLine(int sec); //wait until \n or \r is received
const char *readBufFindWord(const char *word);
const char *waitWordTimeout(const char *word, int sec);
const char *waitWordStopWordTimeout(const char *word, const char *termword, int sec);

unsigned int SendStr(const char *);
unsigned int SendChar(const char);

bool initIP();
bool termIP();
bool HTTP_getFromURL(const char *url);
bool initGPS();
} // namespace SIM868Com

#endif
