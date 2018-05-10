/*
 *	ST32F103xb memory setup.
 *	change this as in the ld file
 *	MEMORY
 *	{
 *		flash0  : org = 0x08000000, len = 120k
 *		flash1  : org = 0x08000000+120k, len = 8k
 *		flash2  : org = 0x00000000, len = 0
 *		flash3  : org = 0x00000000, len = 0
 *		flash4  : org = 0x00000000, len = 0
 *		flash5  : org = 0x00000000, len = 0
 *		flash6  : org = 0x00000000, len = 0
 *		flash7  : org = 0x00000000, len = 0
 *		ram0    : org = 0x20000000, len = 20k
 *		ram1    : org = 0x00000000, len = 0
 *		ram2    : org = 0x00000000, len = 0
 *		ram3    : org = 0x00000000, len = 0
 *		ram4    : org = 0x00000000, len = 0
 *		ram5    : org = 0x00000000, len = 0
 *		ram6    : org = 0x00000000, len = 0
 *		ram7    : org = 0x00000000, len = 0
 *	}
 */

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

//these must be unsigned
#define FLASH_PAGE_COUNT 128U
#define FLASH_PAGE_SIZE 1024U

/**
 * @brief number of pages reserved at the end of flash memory for storing data
 * 
 */
#define FLASH_STORAGE_PAGE_COUNT 8U

namespace flashStorage
{
/**
 * @brief the struct which should contains all data the flash contains with the same memory mapping 
 * 
 */
static struct flashStorageContent_t
{
	uint32_t deviceID = 1234;
} content;

bool writeFlashAll();
bool readFlashAll();
} // namespace flashStorage
