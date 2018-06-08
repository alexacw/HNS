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

namespace GeoPostProcess
{
double originLatitude,
    originLongitude;

char lastSeen[19] = {0};

/*
 * The structure specifies the size of the matrix and then points to
 * an array of data.  The array is of size <code>numRows X numCols</code>
 * and the values are arranged in row order.  That is, the
 * matrix element (i, j) is stored at:
 * <pre>
 *     pData[i*numCols + j]
 * </pre>
 */

arm_matrix_instance_f32 xo;
float32_t xodata[4] = {0,
                       0,
                       0,
                       0};

//estimation covariance 4x4 matrix
arm_matrix_instance_f32 P;
float32_t Pdata[16] = {1, 0, 0, 0,
                       0, 1, 0, 0,
                       0, 0, 1, 0,
                       0, 0, 0, 1};

//Q (4x4 untracked noise coveriance matrix)
//arbitrary diagonal matrix, maybe variance only in velocities
arm_matrix_instance_f32 Q;
const float32_t Qdata[16] = {0, 0, 0, 0,
                             0, 0, 0, 0,
                             0, 0, 0.1, 0,
                             0, 0, 0, 0.1};

//H (measurement matrix):
//1,0,t,0
//0,1,0,t
arm_matrix_instance_f32 H;
float32_t Hdata[8] = {1, 0, 0, 0,
                      0, 1, 0, 0};

//F matrix (state transition)
//1,0,t,0
//0,1,0,t
//0,0,1,0
//0,0,0,1
arm_matrix_instance_f32 F;
float32_t Fdata[16] = {1, 0, 0, 0,
                       0, 1, 0, 0,
                       0, 0, 1, 0,
                       0, 0, 0, 1};

//R (measuremnt covarience)
//let s = HDOP^2/2
//s,0
//0,s
arm_matrix_instance_f32 R;
float32_t Rdata[4] = {1, 0,
                      0, 1};

//No control here
//K = HPHt(HPHt+R)^-1

void initValues(const double &latitude, double longitude, double HDOP, const char *timedate_str)
{
    strcpy(lastSeen, timedate_str);
    originLatitude = latitude;
    originLongitude = longitude;

    //initiialize x and P
    arm_mat_init_f32(&xo, 4, 1, xodata);
    memset(xodata, 0, sizeof(float32_t) * 4);

    arm_mat_init_f32(&P, 4, 4, Pdata);
    memset(Pdata, 0, sizeof(float32_t) * 16);
    Pdata[0] = Pdata[5] = Pdata[10] = Pdata[15] = HDOP * HDOP;

    arm_mat_init_f32(&R, 2, 2, Rdata);
    arm_mat_init_f32(&F, 4, 4, Fdata);
    arm_mat_init_f32(&H, 2, 4, Hdata);
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
                    //update F H and R with dt and HDOP
                    Fdata[2] = Fdata[7] = Hdata[2] = Hdata[7] = dt;
                    Rdata[0] = Rdata[3] = HDOP * HDOP / 2;

                    //TODO: Prediction stage things

                    //find relative x y in terms of origin (first fixed point)
                    //appriximate longitude direction (x) simply by partial derivative at the plane drawn from origin
                    //z (measuremnt): <x,y>
                    arm_matrix_instance_f32 z;
                    float32_t zdata[2] = {(latitude - originLatitude) / M_PI / 2.0 * EARTH_RADIUS,
                                          (longitude - originLongitude) / M_PI / 2.0 * EARTH_RADIUS * cos(originLatitude / M_PI / 2.0)};
                    arm_mat_init_f32(&z, 2, 1, zdata);

                    //find H's transpose
                    static arm_matrix_instance_f32 Ht;
                    static float32_t Ht_data[8] = {0};
                    arm_mat_init_f32(&Ht, 4, 2, Ht_data);

                    arm_mat_trans_f32(&H, &Ht);

                    //find H*P
                    static arm_matrix_instance_f32 HP;
                    static float32_t HP_data[8] = {0};
                    arm_mat_init_f32(&Ht, 2, 4, HP_data);
                    arm_mat_mult_f32(&H, &P, &HP);

                    //find H*P*Ht
                    static arm_matrix_instance_f32 HPHt;
                    static float32_t HPHt_data[4] = {0};
                    arm_mat_init_f32(&Ht, 2, 2, HPHt_data);
                    arm_mat_mult_f32(&HP, &Ht, &HPHt);

                    //find H*P*Ht + R
                    arm_mat_add_f32(&HPHt, &R, &HPHt);
                    //find inverse(H*P*Ht + R) (2x2)
                    arm_mat_inverse_f32(&HPHt, &HPHt);

                    //find Ht * inverse(H*P*Ht + R), no need Ht anymore so use Ht's memory
                    arm_mat_mult_f32(&Ht, &HPHt, &Ht);

                    //calculate Kalman gain (4X2) continue to use Ht's memory
                    arm_mat_mult_f32(&P, &Ht, &Ht);
                    arm_matrix_instance_f32 &K = Ht;

                    //start newP = Po - KHPo
                    static arm_matrix_instance_f32 KHP;
                    static float32_t KHP_data[16] = {0};
                    arm_mat_init_f32(&KHP, 4, 4, KHP_data);
                    //calculate KH
                    arm_mat_mult_f32(&K, &H, &KHP);
                    //calculate KHP
                    arm_mat_mult_f32(&KHP, &P, &KHP);
                    //update newP = Po - KHP
                    arm_mat_sub_f32(&P, &KHP, &P);


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
