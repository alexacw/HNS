#include "can.hpp"
#include "stm32f101xb.h"

namespace canbus
{

static const CANConfig cancfg = {
	.mcr = CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
	.btr = CAN_BTR_BRP(3) | CAN_BTR_SJW(1) | CAN_BTR_TS1(5) | CAN_BTR_TS2(4)};

void init()
{
	palSetPadMode(GPIOA, GPIOA_CAN_RX, PAL_MODE_INPUT);
	palSetPadMode(GPIOA, GPIOA_CAN_TX, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
	canSTM32SetFilters(&CAND1, 0, 0, NULL); //no filter
	canStart(&CAND1, &cancfg);
};

THD_WORKING_AREA(listener_wa, 128);

//Serial listenser
static THD_FUNCTION(listener, arg)
{
	(void)arg;
	static CANRxFrame rxbuf;
	chRegSetThreadName("canbuslistener");

	while (!chThdShouldTerminateX())
	{
		if (canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rxbuf, TIME_INFINITE) == MSG_OK)
		{
		}
	}
};

} // namespace canbus