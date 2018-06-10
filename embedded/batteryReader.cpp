/**
 * @brief 
 * 
 * @file batteryReader.cpp
 * @author Alex Au
 * @date 2018-06-09
 */
#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "stm32f103xb.h"
#include "chprintf.h"
#include "shell.h"
#include "board.h"
#include "usbcfg.h"
namespace BatteryReader
{
// Lets configure our ADC first

// ADCConfig structure for stm32 MCUs is empty
static ADCConfig adccfg = {};

// Create buffer to store ADC results. This is
// one-dimensional interleaved array
#define ADC_BUF_DEPTH 1										// depth of buffer
#define ADC_CH_NUM 1										// number of used ADC channels
static adcsample_t samples_buf[ADC_BUF_DEPTH * ADC_CH_NUM]; // results array

// Fill ADCConversionGroup structure fields
static ADCConversionGroup adccgp =
	{FALSE,					 //circular buffer? no
	 (uint16_t)(ADC_CH_NUM), // number of channels
	 NULL,					 // callback function
	 NULL,					 //Error callback
	 // following stm32 specific fields. Refer to reference manual
	 // CR1 register content
	 0,
	 // CR2 register content
	 0,
	 // SMRP1 register content
	 0,
	 // SMRP2 register content
	 0,
	 // SQR1 register content. Set channel sequence length
	 ADC_SQR1_NUM_CH(ADC_CH_NUM),
	 // SQR2 register content, set to zero for begin
	 0,
	 // SQR3 register content. selected PB0 (channel 8)
	 // SQ1[4:0]: 1st conversion in regular sequence
	 ADC_CHANNEL_IN8};

// Thats all with configuration

uint16_t getADC()
{
	adcStartConversion(&ADCD1, &adccgp, &samples_buf[0], ADC_BUF_DEPTH);
	chThdSleepMilliseconds(10);
	adcStopConversion(&ADCD1);
	return samples_buf[0];
}

bool isBatteryLow()
{
	adcStartConversion(&ADCD1, &adccgp, &samples_buf[0], ADC_BUF_DEPTH);
	chThdSleepMilliseconds(10);
	adcStopConversion(&ADCD1);
	if (samples_buf[0] < 2400)
	{

		chprintf((BaseSequentialStream *)&SDU1, "battery low");
		return true;
	}
	else
		return false;
}

void init()
{

	// Setup pins of our MCU as analog inputs
	palSetPadMode(GPIOB, 0, PAL_MODE_INPUT_ANALOG); // this is 8th channel
	// Following 3 functions use previously created configuration
	// to initialize ADC block, start ADC block and start conversion.
	// &ADCD1 is pointer to ADC driver structure, defined in the depths of HAL.
	// Other arguments defined ourself earlier.
	adcInit();
	adcStart(&ADCD1, &adccfg);
}

} // namespace BatteryReader
