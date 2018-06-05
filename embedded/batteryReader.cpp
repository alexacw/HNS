#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header

// Lets configure our ADC first

// ADCConfig structure for stm32 MCUs is empty
static ADCConfig adccfg = {};

// Create buffer to store ADC results. This is
// one-dimensional interleaved array
#define ADC_BUF_DEPTH 2										// depth of buffer
#define ADC_CH_NUM 1										// number of used ADC channels
static adcsample_t samples_buf[ADC_BUF_DEPTH * ADC_CH_NUM]; // results array

// Fill ADCConversionGroup structure fields
static ADCConversionGroup adccg = {
	// this 3 fields are common for all MCUs
	// set to TRUE if need circular buffer, set FALSE otherwise
	FALSE,
	// number of channels
	(uint16_t)(ADC_CH_NUM),
	// callback function, set to NULL for begin
	NULL,
	// Resent fields are stm32 specific. They contain ADC control registers data.
	// Please, refer to ST manual RM0008.pdf to understand what we do.
	// CR1 register content, set to zero for begin
	0,
	// CR2 register content, set to zero for begin
	0,
	// SMRP1 register content, set to zero for begin
	0,
	// SMRP2 register content, set to zero for begin
	0,
	// SQR1 register content. Set channel sequence length
	((ADC_CH_NUM - 1) << 20),
	// SQR2 register content, set to zero for begin
	0,
	// SQR3 register content. selected PB0 (channel 8)
	// SQ1[4:0]: 1st conversion in regular sequence
	(8),
};

// Thats all with configuration

void adcfunc()
{
	// Setup pins of our MCU as analog inputs
	palSetPadMode(GPIOB, 0, PAL_MODE_INPUT_ANALOG); // this is 15th channel
	// Following 3 functions use previously created configuration
	// to initialize ADC block, start ADC block and start conversion.
	// &ADCD1 is pointer to ADC driver structure, defined in the depths of HAL.
	// Other arguments defined ourself earlier.
	adcInit();
	adcStart(&ADCD1, &adccfg);
	adcStartConversion(&ADCD1, &adccg, &samples_buf[0], ADC_BUF_DEPTH);

	// Thats all. Now your conversion run in background without assistance.

	uint16_t i = 0;

	while (TRUE)
	{
		i = samples_buf[0] / 2;
	}
}