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
#include "flash.hpp"

#define FlASH_START_ADDRESS (0x08000000U + (FLASH_PAGE_COUNT - FLASH_STORAGE_PAGE_COUNT) * FLASH_PAGE_SIZE) //the "()" is imoprtant

#define flashWaitWhileBusy()             \
	{                                    \
		while (FLASH->SR & FLASH_SR_BSY) \
		{                                \
		}                                \
	}

#define flashLock()                 \
	{                               \
		FLASH->CR |= FLASH_CR_LOCK; \
	}

static void flashUnlock(void)
{
	//check if locked
	while (FLASH->CR & FLASH_CR_LOCK)
	{
		//unlock sequence
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
		//check unlocked
		if (!(FLASH->CR & FLASH_CR_LOCK))
			return;
	}
}

flashStorage::flashStorageContent_t flashStorage::content = {12345678,
															 "12345678",
															 "user@mail.com"};

bool flashStorage::writeFlashAll()
{
	flashUnlock();

	flashWaitWhileBusy();
	//start page erase
	FLASH->CR |= FLASH_CR_PER_Msk;

	int pageCount = (sizeof(content) / FLASH_PAGE_SIZE) + 1;

	static uint16_t *flashPtr = (uint16_t *)FlASH_START_ADDRESS;
	static int i = 0;
	for (i = 0; i < pageCount; i++)
	{
		FLASH->AR = (uint32_t)flashPtr;
		FLASH->CR |= FLASH_CR_STRT_Msk;
		flashWaitWhileBusy();
		flashPtr += FLASH_PAGE_SIZE / 2;
	}

	FLASH->CR &= ~FLASH_CR_PER_Msk;

	//start programming
	FLASH->CR |= FLASH_CR_PG_Msk;

	flashPtr = (uint16_t *)FlASH_START_ADDRESS;
	uint16_t *dataPtr = (uint16_t *)&content;
	//calculate how many 16bit write is required
	int dataCount = sizeof(content) / 2;
	for (int i = 0; i < dataCount; i++)
	{
		*flashPtr = *dataPtr;
		flashWaitWhileBusy();
		dataPtr++;
		flashPtr++;
	}

	//handle the 1 byte not programmed
	if (sizeof(content) % 2)
	{
		*(uint8_t *)flashPtr = *(uint8_t *)dataPtr;
		flashWaitWhileBusy();
	}

	FLASH->CR &= ~FLASH_CR_PG;

	flashLock();

	//verify content
	if (memcmp((void *)FlASH_START_ADDRESS, &content, sizeof(content)))
		return false;
	else
		return true;
};

bool flashStorage::readFlashAll()
{
	if (*(char *)FlASH_START_ADDRESS != 0xff)
	{
		content = *(flashStorageContent_t *)FlASH_START_ADDRESS;
		return true;
	}
	else
	{
		writeFlashAll();
		return false;
	}
};
