/**
 *  @file    Area.h
 *  @author
 *  @date    05 June 2018
 */

#pragma once

#include <array>
#include "TypeDef.h"

#define AREA_SIZE 64

struct Area
{
    u32 x, y;
    float amplitude;
    float global_amplitude;
    float frequency;
    u8 octaves;

    std::array<int, 512> permutation;

    Area() : x(0), y(0){};
    Area(u32 x, u32 y);
};
