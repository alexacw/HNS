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
#include <stdlib.h>
#include "arm_math.h"

#define INITIAL_V_COV 0.1
#define UNCERTAIN_V_COV_MULT 0.1

namespace GeoPostProcess
{

//the date/time mesage last received
char lastSeen[19] = {0};

double previousStateX[2];
double previousStateY[2];

double previousP_X[2][2];
double previousP_Y[2][2];

double originLatitude,
    originLongitude;

void initValues(const double &latitude, double longitude, double HDOP, const char *timedate_str)
{
    strcpy(lastSeen, timedate_str);
    originLatitude = latitude;
    originLongitude = longitude;

    previousStateX[0] = previousStateX[1] = {0};
    previousP_X[0][0] = previousP_X[1][0] = previousP_X[0][1] = 0;
    previousP_X[1][1] = INITIAL_V_COV;

    previousStateY[0] = previousStateY[1] = {0};
    previousP_Y[0][0] = previousP_Y[1][0] = previousP_Y[0][1] = 0;
    previousP_Y[1][1] = INITIAL_V_COV;
}

bool update(const char *latitude_str, const char *longitude_str, const char *HDOP_str, const char *timedate_str)
{
    static double latitude, longitude, HDOP;
    if (((latitude = atof(latitude_str)) > 0.0) &&
        ((longitude = atof(longitude_str)) > 0.0) &&
        ((HDOP = atof(HDOP_str)) > 0.0))
    {

        if (!(*lastSeen)) //no previous location data
        {
            //first data
            initValues(latitude, longitude, HDOP, timedate_str);
        }
        else
        {
            if (strncmp(timedate_str, lastSeen, 8) != 0)
            {
                //if the lastseen is in yesterday just dump it for now
                initValues(latitude, longitude, HDOP, timedate_str);
            }
            else
            {
                //this part is very hard coded but seems to be the most efficient lol
                //find dt in second
                double dt = 0;
                double r = HDOP * HDOP / 2;

                dt += timedate_str[8] - lastSeen[8];
                dt *= 10.0; //dt now in hrs
                dt += timedate_str[9] - lastSeen[9];
                dt *= 6.0; //dt now in 10minutes
                dt += timedate_str[10] - lastSeen[10];
                dt *= 10.0; //dt now in minutes
                dt += timedate_str[11] - lastSeen[11];
                dt *= 6.0; //dt now in 10seconds
                dt += timedate_str[12] - lastSeen[12];
                dt *= 10.0; //dt now in seconds
                dt += timedate_str[13] - lastSeen[13];
                dt *= 10.0; //dt now in 100ms
                dt += timedate_str[15] - lastSeen[15];
                dt *= 10.0; //dt now in 10ms
                dt += timedate_str[16] - lastSeen[16];
                dt *= 10.0; //dt now in 1ms
                dt += timedate_str[17] - lastSeen[17];
                dt /= 1000.0; //dt now in seconds

                if (dt < 0)
                {
                    //this should not happen, reset filter anyway
                    initValues(latitude, longitude, HDOP, timedate_str);
                }
                else
                {

                    double measurementX = (latitude - originLatitude) / M_PI / 2.0 * EARTH_RADIUS;
                    double measurementY = (longitude - originLongitude) / M_PI / 2.0 * EARTH_RADIUS * cos(originLatitude / M_PI / 2.0);

                    //Prediction: update previousState and previousP to the prediction
                    previousStateX[0] = dt * previousStateX[1];

                    previousP_X[0][0] += dt * (previousP_X[0][1] + previousP_X[1][0]) + dt * dt * previousP_X[1][1];
                    previousP_X[0][1] += dt * previousP_X[1][1];
                    previousP_X[1][0] += dt * previousP_X[1][1];
                    previousP_X[1][1] += UNCERTAIN_V_COV_MULT * dt;

                    previousStateY[0] = dt * previousStateY[1];

                    previousP_Y[0][0] += dt * (previousP_Y[0][1] + previousP_Y[1][0]) + dt * dt * previousP_Y[1][1];
                    previousP_Y[0][1] += dt * previousP_Y[1][1];
                    previousP_Y[1][0] += dt * previousP_Y[1][1];
                    previousP_Y[1][1] += UNCERTAIN_V_COV_MULT * dt;

                    //need to calculate K = PHt(HPHt+R)^-1
                    //let a = HPHt+R
                    //double aX = previousP_X[0][0] + dt * previousP_X[1][0] + dt * previousP_X[0][1] + dt * dt * previousP_X[1][1] + HDOP * HDOP / 2.0;
                    double Kx1 = (previousP_X[0][0]) / (previousP_X[0][0] + r);
                    double Kx2 = (previousP_X[1][0]) / (previousP_X[0][0] + r);

                    //double aY = previousP_Y[0][0] + dt * previousP_Y[1][0] + dt * previousP_Y[0][1] + dt * dt * previousP_Y[1][1] + HDOP * HDOP / 2.0;
                    double Ky1 = (previousP_Y[0][0]) / (previousP_Y[0][0] + r);
                    double Ky2 = (previousP_Y[1][0]) / (previousP_Y[0][0] + r);

                    //update previous state with estimation
                    previousStateX[0] += Kx1 * (measurementX - previousStateX[0]);
                    previousStateX[1] += Kx2 * (measurementX - previousStateX[0]);

                    previousStateY[0] += Ky1 * (measurementY - previousStateY[0]);
                    previousStateY[1] += Ky2 * (measurementY - previousStateY[0]);

                    
                }
            }
        }
    }
    else
    {
        return false;
    }
}
} // namespace GeoPostProcess
