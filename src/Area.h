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
    float amplitude;
    float frequency;
    u8 octaves;

	std::array<int, 512> permutation;

	Area();
};
