/**
 *  @file    Area.h
 *  @author
 *  @date    05 June 2018
 */

#pragma once

#include <array>
#include "TypeDef.h"

#define AREA_SIZE 64

/**
 * An Area holds certain terrain values like permutation, amplitude, frequency and octaves which are randomly generated.
 * Chunks reference areas to render their values using the before mentioned parameters.
 *
 * \author VM Ware
 * \date 19.06.2018
 */

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
