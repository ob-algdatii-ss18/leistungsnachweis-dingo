/**
*  @file    Perlin.h
*  @author  
*  @date    05 June 2018
*/

#pragma once

#include "Area.h"

/**
 * Perlin fastfloor function to floor a float value.
 *
 * \param The float to be floored.
 *
 * \return Floored float as int.
 */

int perlin_fastfloor(float a);

/**
 * Calculates the perlin noise value of a certain point given coordinates and an area containing other parameters as mentioned below.
 *
 * \param x    The x coordinate.
 * \param y    The y coordinate.
 * \param z    The z coordinate.
 * \param area The area which holds parameters like octaves, amplitude and frequency.
 *
 * \return A float.
 */

float octavePerlin(float x, float y, float z, const Area& area);
