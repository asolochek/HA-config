#include <math.h>

float f_to_c(float tempF)
{
    return ((tempF - 32) / 1.8);
}

float c_to_f(float tempC)
{
    return (tempC * 1.8 + 32);
}

float c_to_k(float tempC)
{
    return tempC + 273.15;
}

float f_to_k(float tempF)
{
    return f_to_c(tempF) + 273.15;
}

float c_to_r(float tempC)
{
    return (c_to_k(tempC) * 1.8);
}

float psia_to_mPa(float psia)
{
    return psia * 68.9476;
}

float psia_to_pa(float psia)
{
    return psia * 6894.76;
}

float compute_dew_point(float tempC, float humidity)
{
    // const float a = 17.625;
    // const float b = 243.04;
    // float p = exp(humidity / 100.0) + a * tempC / (b + tempC);
    // return (b * p) / (a - p);

    const float a = 17.269;
    const float b = 237.3;
    float p = log(humidity / 100.0) + a * tempC / (b + tempC);
    return (b * p) / (a - p);
}

float dew_point_to_RH(float tempC, float dpC)
{
  const float a = 17.625;
  const float b = 243.04;

  float rh = 100 * (exp(a * dpC / (b + dpC)) / exp(a * tempC / (b + tempC)));
  return rh;
}

float sat_pressure(float tempC)
{
    /// Using equation2 from https://www.engineersedge.com/calculators/water_vapor_saturation_pressure_15730.htm
    auto A = -1.0440397e4;
    auto B = -1.1294650e1;
    auto C = -2.7022355e-2;
    auto D = 1.2890360e-5;
    auto E = -2.478068e-9;
    auto F = 6.5459673;

    float tempR = c_to_r(tempC);

    float sat_pressure_psia = exp(A / tempR + B + C * tempR + D * powf(tempR, 2) + E * powf(tempR, 3) + F * log(tempR));
    return psia_to_pa(sat_pressure_psia);
}

float correct_humidity(float temperature1, float humidity1, float temperature2)
{
    float temp1_k = c_to_k(temperature1);
    float temp2_k = c_to_k(temperature2);
    float humidity2 = humidity1 * sat_pressure(temperature1) * temp2_k / (sat_pressure(temperature2) * temp1_k);

    ESP_LOGD("custom", "correcting %.2f%% @ %.2fºC to %.2f%% @ %.2fºC", humidity1, temperature1, humidity2, temperature2);
    return humidity2;
}