/**
 * @brief Kalman filter using latitude longitude and time date as input
 *  estimate with constant velocity motion model
 * 
 * @file GeoPostProcess.hpp
 * @author Alex Au
 * @date 2018-06-03
 */
#include "ch.h"
#include "hal.h"

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"
#include "stm32f1xx.h"

namespace GeoPost
{
void reset();

/**
 * @brief feed the Kalman filter a new measurement
 * 
 * @param latitude 
 * @param longitude 
 * @param HDOP  the vertical 
 * @param timepoint a string of time/date with format yyyyMMddhhmmss.sss, same as that received from the module
 */
void update(const double &latitude, double longitude, double HDOP, const char *timepoint);

/**
 * @brief Get the Estimate object
 * 
 * @return true when estimate is available
 * @return false when no data was given
 */
bool getEstimate(double &latitude, double &longitude);
} // namespace GeoPost