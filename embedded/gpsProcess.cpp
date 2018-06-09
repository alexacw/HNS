/**
 * @brief Kalman filter using latitude longitude and time date as input
 *  estimate with constant velocity motion model
 * 
 * @file GeoPostProcess.cpp
 * @author Alex Au
 * @date 2018-06-03
 */

#include "gpsProcess.hpp"
#include <math.h>
#include <stdlib.h>
#include "constantVKalman.hpp"
#include <math.h>

#define EXTERNAL_V_VAR_PER_UPDATE 2.0

namespace GeoPost
{

//the date/time mesage last received
char lastSeen[19] = {0};
KV_Kalman x_filter;
KV_Kalman y_filter;
double originLatitude, originLongitude;
double estimateLatitude, estimateLongitude;
double long_multi;

bool update(const char *latitude_str, const char *longitude_str, const char *HDOP_str, const char *timedate_str)
{
    static double latitude, longitude, HDOP;
    if (((latitude = atof(latitude_str)) > 0.0) &&
        ((longitude = atof(longitude_str)) > 0.0) &&
        ((HDOP = atof(HDOP_str)) > 0.0) &&
        (strlen(timedate_str) == 18))
    {

        if (!(*lastSeen)) //no previous location data
        {
            //first data
            init(latitude, longitude, timedate_str);
        }
        else
        {
            if (strncmp(timedate_str, lastSeen, 8) != 0)
            {
                //if the lastseen is not in the same date just dump it for now
                init(latitude, longitude, timedate_str);
            }
            else
            {
                //find dt in second
                double dt = deltaTimeFinder(timedate_str, lastSeen);
                double r = HDOP * HDOP / 2;

                if (dt < 0)
                {
                    //this should not happen, reinit anyway
                    init(latitude, longitude, timedate_str);
                }
                else
                {
                    //TODO: perform filtering
                    double measurementX = (latitude - originLatitude) / M_PI / 2.0 * EARTH_RADIUS;
                    double measurementY = (longitude - originLongitude) / M_PI / 2.0 * EARTH_RADIUS * long_multi;
                    double est_x = x_filter.correct(dt, measurementX, r, EXTERNAL_V_VAR_PER_UPDATE);
                    double est_y = y_filter.correct(dt, measurementY, r, EXTERNAL_V_VAR_PER_UPDATE);
                    estimateLatitude = originLatitude + est_x / EARTH_RADIUS * 2.0 * M_PI;
                    estimateLongitude = originLongitude + est_y / EARTH_RADIUS * 2.0 * M_PI / long_multi;
                }

                strcpy(lastSeen, timedate_str);
            }
        }
    }
    else
    {
        return false;
    }
};

bool getEstimate(double &latitude, double &longitude)
{

    latitude = estimateLatitude;
    longitude = estimateLongitude;
    return (*lastSeen);
};

void init(const double &originLat, const double &originLong, const char *timedate_str)
{
    strcpy(lastSeen, timedate_str);
    x_filter.init(0, EXTERNAL_V_VAR_PER_UPDATE);
    y_filter.init(0, EXTERNAL_V_VAR_PER_UPDATE);
    estimateLatitude = originLatitude = originLat;
    estimateLongitude = originLongitude = originLong;
    long_multi = std::cos(originLatitude / M_PI / 2.0);
};

double deltaTimeFinder(const char *timedate_str_new, const char *timedate_str_old)
{
    double dt = 0;
    dt += timedate_str_new[8] - timedate_str_old[8];
    dt *= 10.0; //dt now in hrs
    dt += timedate_str_new[9] - timedate_str_old[9];
    dt *= 6.0; //dt now in 10minutes
    dt += timedate_str_new[10] - timedate_str_old[10];
    dt *= 10.0; //dt now in minutes
    dt += timedate_str_new[11] - timedate_str_old[11];
    dt *= 6.0; //dt now in 10seconds
    dt += timedate_str_new[12] - timedate_str_old[12];
    dt *= 10.0; //dt now in seconds
    dt += timedate_str_new[13] - timedate_str_old[13];
    dt *= 10.0; //dt now in 100ms
    dt += timedate_str_new[15] - timedate_str_old[15];
    dt *= 10.0; //dt now in 10ms
    dt += timedate_str_new[16] - timedate_str_old[16];
    dt *= 10.0; //dt now in 1ms
    dt += timedate_str_new[17] - timedate_str_old[17];
    dt /= 1000.0; //dt now in seconds

    return dt;
};
} // namespace GeoPost
