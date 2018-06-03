/**
 * @brief Kalman filter using latitude longitude and time date as input
 *  estimate with constant velocity motion model
 * 
 * @file GeoPostProcess.cpp
 * @author Alex Au
 * @date 2018-06-03
 */

#include "GeoPostProcess.hpp"
#include <math.h>

namespace GeoPostProcess
{
double originLatitude,
    originLongitude;
//x vector (state)
double
    estimate_X,
    estimate_Y,
    estimate_VX,
    estimate_VY;

//P covarience matrix
double
    var_X,
    var_Y,
    var_VX,
    var_VY;

char lastSeen[19] = {0};

//F matrix (state transition)
//1,0,t,0
//0,1,0,t
//0,0,1,0
//0,0,0,1
//Q (untracked noise coveriance matrix)
//arbitrary diagonal matrix
//No control here
//H (measurement matrix):
//1,0,t,0
//0,1,0,t
//z (measuremnt): <x,y>
//R (measuremnt covarience)
//let s = HDOP^2/2
//R=
//s,0
//0,s
//K = HPHt(HPHt+R)^-1

void initValues(const double &latitude, double longitude, double HDOP, const char *timepoint)
{
    strcpy(lastSeen, timepoint);
    originLatitude = latitude;
    originLongitude = longitude;
    estimate_VX = estimate_VY = estimate_X = estimate_Y = 0;
    var_X = var_Y = var_VX = var_VY = HDOP * HDOP;
}

void update(const double &latitude, double longitude, double HDOP, const char *timepoint)
{
    if (!(*lastSeen)) //no previous location data
    {
        //first data
        initValues(latitude, longitude, HDOP, timepoint);
    }
    else
    {
        //this part is very hard coded but seems to be the most efficient lol
        if (strncmp(timepoint, lastSeen, 8) != 0)
        {
            //if the lastseen is in yesterday just dump it for now
            initValues(latitude, longitude, HDOP, timepoint);
        }
        else
        {
            //find dt in second
            double dt = 0;
            dt += timepoint[8] - lastSeen[8];
            dt *= 10.0; //dt now in hrs
            dt += timepoint[9] - lastSeen[9];
            dt *= 6.0; //dt now in 10minutes
            dt += timepoint[10] - lastSeen[10];
            dt *= 10.0; //dt now in minutes
            dt += timepoint[11] - lastSeen[11];
            dt *= 6.0; //dt now in 10seconds
            dt += timepoint[12] - lastSeen[12];
            dt *= 10.0; //dt now in seconds
            dt += timepoint[13] - lastSeen[13];
            dt *= 10.0; //dt now in 100ms
            dt += timepoint[15] - lastSeen[15];
            dt *= 10.0; //dt now in 10ms
            dt += timepoint[16] - lastSeen[16];
            dt *= 10.0; //dt now in 1ms
            dt += timepoint[17] - lastSeen[17];
            dt /= 1000.0; //dt now in seconds

            if (dt < 0)
            {
                //this should not happen, reset filter anyway
                initValues(latitude, longitude, HDOP, timepoint);
            }
            else
            {
                //find relative x y in terms of origin
                //appriximate longitude direction (x) simply by partial derivative at the plane drawn from origin
                double measure_y = (latitude - originLatitude) / M_PI / 2.0 * EARTH_RADIUS;
                double measure_x = (longitude - originLongitude) / M_PI / 2.0 * EARTH_RADIUS * cos(originLatitude / M_PI / 2.0);
                //TODO: Kalman filter
            }
        }
    }
}
} // namespace GeoPostProcess
