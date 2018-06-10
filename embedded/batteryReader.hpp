/**
 * @brief 
 * 
 * @file batteryReader.hpp
 * @author Alex Au
 * @date 2018-06-09
 */
#include "ch.h"
namespace BatteryReader
{
void init();
bool isBatteryLow();
uint16_t getADC();

} // namespace BatteryReader