
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
extern bool monitorSerial;
extern bool outBound;
extern bool aggressive;
extern bool receivedCall;
extern bool receivedNewSMS;
extern bool calmDownAlert;
void initSerial();
void startSerialRead();
void stopSerialRead();
void initModulePara();
void readBufInit();
void readBufclear();

//search the buffer for incoming message and calls, and react to them
void findandactKeywords();

void readBuffedMsg();
bool readBufWaitLine(int sec); //wait until \n or \r is received
const char *readBufFindWord(const char *word);
const char *waitWordTimeout(const char *word, int sec);
const char *waitWordStopWordTimeout(const char *word, const char *termword, int sec);

unsigned int SendStr(const char *);
unsigned int SendChar(const char);

bool initGPRS();
bool deinitGPRS();
bool checkGPRS();
bool HTTP_getFromURL(const char *url);
//call after get from url success
bool HTTP_getLocStatus();
bool HTTP_postToURL(const char *url);
bool updateGPS();
bool turnoffGPS();
bool updateGSMLoc(double &lat, double &lng);
bool sendSMS(const char *receiverNumber, const char *message);
bool unreadSMSFindSender(const char *receiverNumber);

void reportToSMS(const double &lat, const double &lng);
bool reportToServer(const double &lat, const double &lng);

} // namespace SIM868Com

#endif
