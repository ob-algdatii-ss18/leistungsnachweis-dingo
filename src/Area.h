/**
*  @file    Area.h
*  @author  
*  @date    05 June 2018
*/

#pragma once

#include "TypeDef.h"
#include <array>

#define AREA_SIZE 64

struct Area
{
    // "Area-Koordinaten" ?
	// Sollten mit AREA_SIZE multipliziert werden für absolute Koordinaten
    u8 x;
    u8 y;

    float amplitude;
    float frequency;
    u8 octaves;

	std::array<int, 512> permutation;

	Area(u8 x, u8 y);
};
