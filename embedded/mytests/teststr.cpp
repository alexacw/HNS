#include <stdio.h>

int main()
{
    const char *timedate_str = "20170602235556.123";
    const char *lastSeen = "20170602224556.123";

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
    printf("%f\n", dt);
}