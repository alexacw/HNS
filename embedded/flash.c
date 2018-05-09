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
#include "flash.h"

#define flashLock() { FLASH->CR |= FLASH_CR_LOCK; }

static bool flashUnlock(void)
{
	/* Check if unlock is really needed */
	if (!(FLASH->CR & FLASH_CR_LOCK))
		return true;

	/* unlock sequence */
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;

	/* Check if unlock was successful */
	if (FLASH->CR & FLASH_CR_LOCK)
		return false;
	return true;
}

void flashErase()
{
	/* Unlock flash for write access */
	if (!flashUnlock())
		return;

	/* Wait for any busy flags. */
	flashWaitWhileBusy();

	FLASH->CR |= FLASH_CR_SER;
	FLASH->CR |= FLASH_CR_STRT;

	/* Wait until it's finished. */
	flashWaitWhileBusy();

	/* Sector erase flag does not clear automatically. */
	FLASH->CR &= ~FLASH_CR_SER;
	FLASH->CR &= ~(FLASH_CR_SNB_0 | FLASH_CR_SNB_1 | FLASH_CR_SNB_2 | FLASH_CR_SNB_3);

	FLASH_DataCacheCmd(ENABLE);

	/* Lock flash again */
	flashLock();
}