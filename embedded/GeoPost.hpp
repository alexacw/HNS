/**
 * @brief Kalman filter using latitude longitude and time date as input
 *  estimate with constant velocity motion model
 * 
 * @file GeoPostProcess.hpp
 * @author Alex Au
 * @date 2018-06-03
 */
//sample received text from sim868:
//(send) AT+CGNSINF
//+CGNSINF: 1,1,20180426125307.000,22.338201,114.264096,112.488,0.00,208.1,1,,1.5,1.7,0.8,,12,10,,,33,,
//
//OK

#include "ch.h"
#include "hal.h"

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"
#include "stm32f1xx.h"

#define EARTH_RADIUS 6378100.0 //in meters
namespace GeoPost
{

extern char lastSeen[19];
extern double estimateLatitude;
extern double estimateLongitude;

void reset();

/**
 * @brief feed the Kalman filter a new measurement
 * 
 * @param latitude 
 * @param longitude 
 * @param HDOP  the vertical 
 * @param timepoint a string of time/date with format yyyyMMddhhmmss.sss, same as that received from the module
 * @param latitude  the estimatated latitude on this update
 * @param longitude
 * @return true when 
 */
bool update(const char *latitude_str, const char *longitude_str, const char *HDOP_str, const char *timedate_str);

/**
 * @brief Get the Estimated latitude and longitude when last updated
 * 
 * @return true when estimate is available
 * @return false when no data was given or date expired
 */
bool getEstimate(double &latitude, double &longitude);

/**
 * @brief initialize the tracking with the specified starting point and time
 * 
 * @param originLat 
 * @param originLong 
 * @param timedate_str 
 */
void init(const double &originLat, const double &originLong, const char *timedate_str);

/**
 * @brief find the delta time between two time/date string in yyyyMMddhhmmss.sss format
 * 
 * @param timedate_str_new 
 * @param timedate_str_old 
 * @return double 
 */
double deltaTimeFinder(const char *timedate_str_new, const char *timedate_str_old);
} // namespace GeoPost