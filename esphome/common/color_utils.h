/**
 * @file color_utils.h
 * @author Aaron Solochek <aarons@gmail.com>
 * @brief Functions for color manipulation
 * @version 0.1
 * @date 2023-08-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <stdint.h>
//#include <RgbColor.h>
//#include <HslColor.h>
#include "aqi.h"

#define BLACK       (uint32_t)0x000000 ///< Black
#define WHITE       (uint32_t)0xFFFFFF ///< White
#define DIMWHITE    (uint32_t)0xAAAAAA ///< Dim White
#define YELLOW      (uint32_t)0xFFFF00 ///< Yellow
#define RED         (uint32_t)0xFF0000 ///< Red
#define PURPLE      (uint32_t)0x7F007F ///< Purple
#define ORANGE      (uint32_t)0xFF7F00 ///< Orange
#define MAGENTA     (uint32_t)0xFF00FF ///< Magenta
#define GREEN       (uint32_t)0x00FF00 ///< Green
#define CYAN        (uint32_t)0x00FFFF ///< Cyan
#define BROWN       (uint32_t)0x996633 ///< Brown
#define BLUE        (uint32_t)0x0000FF ///< Blue


Color white(0xff, 0xff, 0xff);
Color green(0x5d, 0xd9, 0x39);
Color yellow(0xff, 0xfb, 0x46);
Color orange(0xee, 0x78, 0x26);
Color red(0xe8, 0x2e, 0x18);
Color purple(0x7a, 0x3f, 0x88);
Color maroon(0x69, 0x15, 0x20);
Color blue(0, 0x80, 0xFF);
Color gray(0x7f, 0x7f, 0x7f);


const breakpoint co2_breaks[] =
{
    {0, 400},
    {401, 600},
    {601, 800},
    {801, 1100},
    {1101, 2000},
    {2001, 2500},
};

Color aqiColors[] = {
    Color(0, 228, 0),
    Color(255, 255, 0),
    Color(255, 126, 0),
    Color(255, 0, 0),
    Color(143, 63, 151),
    Color(126, 0, 35),
    Color(126, 0, 35)
};

Color co2Colors[] = {
    Color(0, 128, 255),
    Color(0, 228, 0),
    Color(255, 255, 0),
    Color(255, 126, 0),
    Color(255, 0, 0),
    Color(143, 63, 151),
    Color(126, 0, 35)
};

Color aqiColor(int aqi) {
    return aqiColors[high_index(aqi, aqi_breaks)];
}

Color co2Color(int co2) {
    return co2Colors[high_index(co2, co2_breaks)];
}

#if 0
RgbColor blend(int val, const breakpoint map[], RgbColor colors[], int map_len) {
    int i = high_index(val, map);
    RgbColor left = colors[i];
    RgbColor right = i < map_len-1 ? colors[i+1] : colors[map_len-1];

    float mix_base = map[i].lo;
    float mix_high = map[i].hi;
    float mix = (val - mix_base) / (mix_high - mix_base);

    HslColor leftHSL(left);
    HslColor rightHSL(right);
    HslColor blend = LinearBlend<HslColor>(leftHSL, rightHSL, mix);
    return RgbColor(blend);
}
#endif

