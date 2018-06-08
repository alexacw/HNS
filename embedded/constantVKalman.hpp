/**
 * @brief Kalman filter algorithm for single axis constant velocity motion with measurement only on position with no control
 * 
 * @file constantVKalman.cpp
 * @author Alex Au
 * @date 2018-06-08
 */
#include <math.h>

class KV_Kalman
{
public:
  KV_Kalman();

  void init(const double &s, const double &var);

  /**
   * @brief feed a measurement, update estimate
   * 
   * @param dt delta time from last correction
   * @param s measured displacement
   * @param var measurment variance
   */
  double correct(const double &dt, const double &measure_s, const double &measure_var, const double &ext_v_var);

  /**
 * @brief 
 * 
 * @param dt delta time from last correction 
 * @return double the predicted displacement
 */
  void predict(const double &dt, const double &ext_v_var);

  const double &getS();
  const double &getV();

private:
  //P matrix
  double cov_ss, cov_sv, cov_vv;
  //states
  double s, v;

  //Prediction temporaries
  //P matrix
  double p_cov_ss, p_cov_sv, p_cov_vv;
  //states
  double p_s, p_v;
};