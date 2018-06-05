/**
*  @file    Chunk.h
*  @author  
*  @date    05 June 2018
*/

#pragma once

#include "TypeDef.h"
#include "Area.h"

#define CHUNK_SIZE 16

struct Chunk
{
	// Beim Rendern als offset benutzen um die absoluten Koordinaten der Werte zu erhalten.
    u8 x;
    u8 y;

	Area areas[4];

	// 1 inner			1 Area
	// 2 vertical		2 Areas (Oben, Unten)
	// 3 horizontal		2 Areas (Rechts, Links)
	// 4 corner			4 Areas (Oben Links, Oben Rechts, Unten Links, Unten Rechts)
    u8 type;

	float values[CHUNK_SIZE * CHUNK_SIZE];

	Chunk(u8 x, u8 y, Area areas[], u8 type);
	void calculate();

	private:

	void calculate_inner();
    void calculate_vertical();
    void calculate_horizontal();
    void calculate_corner();
};
