#include "constantVKalman.hpp"
#include <math.h>
#include <stdlib.h>
#include <chrono>
#include <stdio.h>

int main()
{
    KV_Kalman filter;
    srand(clock());
    filter.init(0, 1);
    double s = 0;
    while (s < 10)
    {
        filter.correct(1, s, 1000, 2);
        printf("measure s: %f displacement: %f velocity: %f\n", s, filter.getS(), filter.getV());
        s += 0.1;
    }
    while (s > 0)
    {
        filter.correct(1, s, 1000, 2);
        printf("measure s: %f displacement: %f velocity: %f\n", s, filter.getS(), filter.getV());
        s -= 0.1;
    }
}