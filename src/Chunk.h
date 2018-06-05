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
    // "Chunk-Koordinaten" ?
    // Sollten mit CUNK_SIZE multipliziert werden für absolute Koordinaten
    u8 x;
    u8 y;

	public:
	// Je nach Anzahl der Areas bestimmt sich der Typ des Chunks
    // 1 = Mittelpunkt
    // 2 = Randpunkt
    // 4 = Eckpunkt
    Area areas[4];

	u8 getAbsoluteX();
    u8 getAbsoluteY();
};
