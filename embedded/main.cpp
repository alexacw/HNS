/*
 ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

/*
 * openocd -f /usr/local/share/openocd/scripts/interface/stlink-v2.cfg -f /usr/local/share/openocd/scripts/target/stm32f1x.cfg
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"
#include "board.h"

#include "usbcfg.h"

#include "SIM868Com.hpp"
#include "flash.hpp"
#include "batteryReader.hpp"
#include "GeoPost.hpp"

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(2048)

static void setDeviceID(BaseSequentialStream *chp, int argc, char *argv[])
{
	if (argc == 1)
	{
		uint32_t tempLong = strtol(argv[0], NULL, 0);
		flashStorage::content.deviceID = tempLong;
		if (tempLong != 0)
		{
			flashStorage::content.deviceID = tempLong;
			if (flashStorage::writeFlashAll())
				chprintf(chp, "write device id to flash success\r\n");
			else
				chprintf(chp, "write to flash failed\r\n");
		}
		else
			chprintf(chp, "invalid input, id must be positive integer\r\n");
	}
	else
	{
		chprintf(chp, "Usage: setID [id (4 char)]\r\n");
		return;
	}
};

static void setTel(BaseSequentialStream *chp, int argc, char *argv[])
{
	if (argc == 1)
	{
		if (strlen(argv[0]) == 8)
		{
			strcpy(flashStorage::content.parentTel, argv[0]);
			if (flashStorage::writeFlashAll())
				chprintf(chp, "write telephone number to flash success\r\n");
			else
				chprintf(chp, "write to flash failed\r\n");
		}
		else
			chprintf(chp, "invalid input, phone number must be 8 digit\r\n");
	}
	else
	{
		chprintf(chp, "Usage: setTel [phone number (8 digit)]\r\n");
		return;
	}
};

static void readDeviceInfo(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	chprintf(chp, "Device ID: %ld\r\n", flashStorage::content.deviceID);
	chprintf(chp, "Parent tel: %s\r\n", flashStorage::content.parentTel);
	chprintf(chp, "Parent email: %s\r\n", flashStorage::content.parentEmail);
	return;
}

static void getADC(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	chprintf((BaseSequentialStream *)chp, "ADC read: %d\r\n", BatteryReader::getADC());
	return;
}

static void send2uart(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	if (argc == 1)
		chprintf((BaseSequentialStream *)&SD1, argv[0]);
	chprintf((BaseSequentialStream *)&SD1, "\r\n");
	return;
}

static void clearSDbuf(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	SIM868Com::readBufclear();
	chprintf((BaseSequentialStream *)chp, "readBufclear() called\n");
	return;
}

static void toggleSerialMonitor(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	SIM868Com::monitorSerial = !SIM868Com::monitorSerial;
	chprintf((BaseSequentialStream *)chp, "toggled Serial Monitor over USB, now %s\n", SIM868Com::monitorSerial ? "on" : "off");
	return;
}

bool trackToggle = true;
static void toggleTracking(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	trackToggle = !trackToggle;
	chprintf((BaseSequentialStream *)chp, "toggled tracking, now %s\n", trackToggle ? "on" : "off");
	return;
}

static void sendSMScmd(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	if (argc == 2)
	{
		chprintf((BaseSequentialStream *)chp, "Sending SMS to %s;\n content:%s\n", argv[0], argv[1]);
		if (SIM868Com::sendSMS(argv[0], argv[1]))
		{
			chprintf((BaseSequentialStream *)chp, "SMS sent");
		}
	}
	else
	{
		chprintf((BaseSequentialStream *)chp, "usage: sendSMS [receiver number] [content]");
	}
	return;
}

static void findSMScmd(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	if (argc == 1)
	{
		chprintf((BaseSequentialStream *)chp, "finding SMS from %s;\n", argv[0]);
		if (SIM868Com::unreadSMSFindSender(argv[0]))
		{
			chprintf((BaseSequentialStream *)chp, "SMS found");
		}
	}
	else
	{
		chprintf((BaseSequentialStream *)chp, "usage: sendSMS [sender number]");
	}
	return;
}

static void httpgetcmd(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	if (argc == 1)
	{
		chprintf((BaseSequentialStream *)&chp, "get from url: %s;\n", argv[0]);
		if (SIM868Com::sendSMS(argv[0], argv[1]))
		{
			chprintf((BaseSequentialStream *)chp, "SMS sent");
		}
	}
}

static void trygsmloc(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	if (argc == 1)
	{
		chprintf((BaseSequentialStream *)chp, "trygsmloc\n");
		double a, b;
		char temp[50];
		SIM868Com::updateGSMLoc(temp, a, b);
	}
}

static void makeaggressive(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	SIM868Com::aggressive = true;
	chprintf((BaseSequentialStream *)chp, "makeaggressive\n");
}

static const ShellCommand commands[] = {
	{"setID", setDeviceID},
	{"setTel", setTel},
	{"Info", readDeviceInfo},
	{"getADC", getADC},
	{"send", send2uart},
	{"clearSDbuf", clearSDbuf},
	{"tSD", toggleSerialMonitor},
	{"track", toggleTracking},
	{"sendSMS", sendSMScmd},
	{"httpgetcmd", httpgetcmd},
	{"findSMS", findSMScmd},
	{"trygsmloc", trygsmloc},
	{"mg", makeaggressive},
	{NULL, NULL}};

static const ShellConfig shell_cfg1 =
	{(BaseSequentialStream *)&SDU1, commands};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/
/*
 * Blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waBlinker, 128);
static __attribute__((noreturn)) THD_FUNCTION(BlinkerThd, arg)
{

	(void)arg;
	chRegSetThreadName("blinker");
	while (!chThdShouldTerminateX())
	{
		systime_t time = serusbcfg.usbp->state == USB_ACTIVE ? 100 : 300;
		palClearPad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(time);
		palSetPad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(time);
	}
}

bool enableGPS = true;
bool serverPresent = true;
bool enableSMS = true;
bool enableEmail = true;

static THD_WORKING_AREA(waSim868Interface, 512);
static __attribute__((noreturn)) THD_FUNCTION(Sim868InterfaceThd, arg)
{

	(void)arg;
	chRegSetThreadName("Sim868Interface");

	SIM868Com::initSerial();
	SIM868Com::startSerialRead();
	SIM868Com::initModulePara();

	flashStorage::readFlashAll();
	while (!SIM868Com::initGPRS())
	{
		chprintf((BaseSequentialStream *)&SDU1, "GPRS init failed");
	}
	int gpsFailCount = 0;
	int aggressiveCount = 0;
	double reportlat, reportlng;
	char timestr[50];
	int sleeptime = 10;
	while (!chThdShouldTerminateX())
	{
		//TODO:
		//aggressive mode trigger, active for at leat 5 minute, 2 sec sleep

		if (SIM868Com::receivedCall || SIM868Com::receivedNewSMS)
		{
			aggressiveCount = 150;
		}

		if (SIM868Com::aggressive)
		{
			aggressiveCount = 150;
			SIM868Com::aggressive = false;
		}

		if (aggressiveCount > 0)
		{
			aggressiveCount--;
			sleeptime = 2;
			enableGPS = true;
		}
		else if (BatteryReader::isBatteryLow())
		{
			sleeptime = 30;
			enableGPS = false;
		}
		else
		{
			//normal mode
			sleeptime = 10;
			enableGPS = true;
		}

		if (trackToggle)
		{
			if (enableGPS)
			{
				if (SIM868Com::updateGPS())
				{
					GeoPost::getEstimate(reportlat, reportlng);
					strcpy(timestr, GeoPost::lastSeen);
					SIM868Com::reportToServer(reportlat, reportlng);
					SIM868Com::HTTP_getLocStatus();
					gpsFailCount = 0;
				}
				else
				{
					gpsFailCount++;
				}
				if (gpsFailCount > 5)
				{
					if (SIM868Com::updateGSMLoc(timestr, reportlat, reportlng))
					{
						SIM868Com::reportToServer(reportlat, reportlng);
						SIM868Com::HTTP_getLocStatus();
					}
				}

				if (SIM868Com::outBound)
				{
					SIM868Com::reportToSMS(GeoPost::lastSeen, reportlat, reportlng);
				}
			}
			else
			{
				gpsFailCount = 0;
				SIM868Com::turnoffGPS();
			}
		}

		chThdSleepSeconds(sleeptime);
	}
}

/*
 * Application entry point.
 */
int main(void)
{

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	/*
	 * Initializes a serial-over-USB CDC driver.
	 */
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);

	/*
	 * Activates the USB driver and then the uint8_tUSB bus pull-up on D+.
	 * Note, a delay is inserted in order to not have to disconnect the cable
	 * after a reset.
	 */
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(100);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	/*
	 * Shell manager initialization.
	 */
	shellInit();

	BatteryReader::init();

	/*
	 * Creates the blinker thread.
	 */
	chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO, BlinkerThd, NULL);

	//create the state machine thread

	chThdCreateStatic(waSim868Interface, sizeof(waSim868Interface), NORMALPRIO, Sim868InterfaceThd, NULL);
	/*
	 * Normal main() thread activity, spawning shells.
	 */
	thread_t *shelltp = NULL;
	while (true)
	{

		//detecting usb connetion and start shell thread if connected
		if (SDU1.config->usbp->state == USB_ACTIVE)
		{
			if (!shelltp)
			{
				shelltp = chThdCreateFromHeap(NULL,
											  SHELL_WA_SIZE, "shell",
											  NORMALPRIO + 1, shellThread, (void *)&shell_cfg1);
			}
			else if (chThdTerminatedX(shelltp))

				shelltp = chThdCreateFromHeap(NULL,
											  SHELL_WA_SIZE, "shell",
											  NORMALPRIO + 1, shellThread, (void *)&shell_cfg1);
		}
		chThdSleepMilliseconds(500);
	}
}
