/**
*  @file    Perlin.h
*  @author  
*  @date    05 June 2018
*/

#pragma once

#include "Area.h"

int perlin_fastfloor(float a);

float octavePerlin(float x, float y, float z, const Area& area);
