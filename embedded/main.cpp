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

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"
#include "board.h"

#include "usbcfg.h"

#include "SIM868Com.hpp"

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(2048)

/* Can be measured using dd if=/dev/xxxx of=/dev/null bs=512 count=10000.*/
static void cmd_write(BaseSequentialStream *chp, int argc, char *argv[])
{
	static uint8_t buf[] =
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

	(void)argv;
	if (argc > 0)
	{
		chprintf(chp, "Usage: write\r\n");
		return;
	}

	while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
	{
		/* Writing in channel mode.*/
		chnWrite(&SDU1, buf, sizeof buf - 1);
	}
	chprintf(chp, "\r\n\nstopped\r\n");
}

static const ShellCommand commands[] = {
	{"write", cmd_write},
	{NULL, NULL}};

static const ShellConfig shell_cfg1 =
	{(BaseSequentialStream *)&SDU1, commands};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/
/*
 * Blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static __attribute__((noreturn)) THD_FUNCTION(Thread1, arg)
{

	(void)arg;
	chRegSetThreadName("blinker");
	while (true)
	{
		systime_t time = serusbcfg.usbp->state == USB_ACTIVE ? 100 : 300;
		palClearPad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(time);
		palSetPad(GPIOC, GPIOC_LED);
		chThdSleepMilliseconds(time);
		static char temp[] = "received ok";
		sdAsynchronousWrite(&SD1, (uint8_t *)temp, sizeof(temp) - 1);
		chnWrite(&SDU1, (uint8_t *)temp, sizeof(temp) - 1);
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
	chThdSleepMilliseconds(500);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	/*
	 * Shell manager initialization.
	 */
	shellInit();

	SIM868Com::initSerial();
	SIM868Com::startSerialRead();

	/*
	 * Creates the blinker thread.
	 */
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
	/*
	 * Normal main() thread activity, spawning shells.
	 */
	while (true)
	{
		if (SDU1.config->usbp->state == USB_ACTIVE)
		{
			thread_t *shelltp = chThdCreateFromHeap(NULL,
													SHELL_WA_SIZE, "shell",
													NORMALPRIO + 1, shellThread, (void *)&shell_cfg1);
			chThdWait(shelltp); /* Waiting termination.             */
		}
		chThdSleepMilliseconds(300);
	}
}
