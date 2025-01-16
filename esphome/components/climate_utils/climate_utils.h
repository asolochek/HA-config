#pragma once


float f_to_c(float tempF);
float c_to_f(float tempC);
float c_to_k(float tempC);
float f_to_k(float tempF);
float c_to_r(float tempC);
float psia_to_mPa(float psia);
float psia_to_pa(float psia);
float rh_to_dewpoint(float tempC, float humidity);
float dewpoint_to_rh(float tempC, float dpC);
float sat_pressure(float tempC);
float correct_humidity(float temperature1, float humidity1, float temperature2);