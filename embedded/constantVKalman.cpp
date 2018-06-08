/**
 * @brief Kalman filter algorithm for single axis constant velocity motion with measurement only on position with no control
 * 
 * @file constantVKalman.cpp
 * @author Alex Au
 * @date 2018-06-08
 */
#include <math.h>
#include "constantVKalman.hpp"

KV_Kalman::KV_Kalman(){};

void KV_Kalman::init(const double &initialDisplacement, const double &var)
{
    cov_ss = cov_vv = var;
    cov_sv = 0;
    s = initialDisplacement;
    v = 0;
};

void KV_Kalman::predict(const double &dt, const double &ext_v_var)
{
    p_s = s + dt * v;
    p_v = v;

    p_cov_ss = cov_ss + dt * 2.0 * cov_sv + dt * dt * cov_vv;
    p_cov_sv = cov_sv + dt * cov_vv;
    p_cov_vv = cov_vv + ext_v_var;
};

double KV_Kalman::correct(const double &dt, const double &measure_s, const double &measure_var, const double &ext_v_var)
{
    //perform prediction on current dt
    predict(dt, ext_v_var);

    double temp = 1.0 / (p_cov_ss + measure_var);
    double mp_diff = measure_s - p_s;
    //estimate new state
    s = p_s + temp * p_cov_ss * (mp_diff);
    v = p_v + temp * p_cov_sv * (mp_diff);
    //estimate new covariance matrix
    cov_sv = p_cov_sv - temp * p_cov_sv * p_cov_ss;
    cov_ss = p_cov_ss - temp * p_cov_ss * p_cov_ss;
    cov_vv = p_cov_vv - temp * p_cov_vv * p_cov_vv;

    return s;
}

const double &KV_Kalman::getS()
{
    return s;
};
const double &KV_Kalman::getV()
{
    return v;
};
