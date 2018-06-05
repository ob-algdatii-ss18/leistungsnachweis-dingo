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

	float values[CHUNK_SIZE * CHUNK_SIZE];

	Chunk(u8 x, u8 y, Area a);
};
