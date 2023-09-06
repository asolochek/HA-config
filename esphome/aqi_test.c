#include "aqi.h"
#include <stdio.h>
#include <string.h>

int main()
{
    int pm25 = 0;
    int pm10 = 0;

    float raw;

    // for (raw = 0.0; raw < 500; raw += 50)
    // {
    //     pm25 = get_pm2_5_aqi(raw);
    //     printf("PM2.5 Concentration: %.1f => AQI: %d\n", raw, pm25);
    // }

    //     pm25 = get_pm2_5_aqi(50.4);
    //     printf("PM2.5 Concentration: %.1f => AQI: %d\n", 50.4, pm25);
    //     pm25 = get_pm2_5_aqi(50.5);
    //     printf("PM2.5 Concentration: %.1f => AQI: %d\n", 50.5, pm25);

    // for (raw = 0.0; raw < 500; raw += 50)
    // {
    //     pm10 = get_pm10_aqi(raw);
    //     printf("PM10 Concentration %.1f => AQI: %d\n", raw, pm10);
    // }

    for (raw = 0.0; raw < .604; raw += .005)
    {
        int h1 = get_1h_ozone_aqi(raw);
        int h8 = get_8h_ozone_aqi(raw);
        int o3 = get_ozone_aqi(raw, raw);
        printf("Ozone raw: %.3f\t1h: %d\t8h: %d\tofficial: %d\n", raw, h1, h8, o3);
    }


    int total = get_total_aqi(35.9, 0, .104, .104, 8.4, 0, 0);
    printf("Total AQI: %d\n", total);
}