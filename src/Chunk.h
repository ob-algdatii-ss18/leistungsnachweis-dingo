/**
*  @file    Chunk.h
*  @author
*  @date    05 June 2018
*/

#pragma once

#include "TypeDef.h"
#include "Area.h"

#define CHUNK_SIZE 64

enum ChunkType : u8
{
	Inner = 0,
	Vertical,
	Horizontal, 
	Corner
};

struct Chunk
{
	// Beim Rendern als offset benutzen um die absoluten Koordinaten der Werte zu erhalten.
	u32 x;
	u32 y;
    
	Area* areas[4];
    
	// 1 inner			1 Area
	// 2 vertical		2 Areas (Oben, Unten)
	// 3 horizontal		2 Areas (Rechts, Links)
	// 4 corner			4 Areas (Oben Links, Oben Rechts, Unten Links, Unten Rechts)
	ChunkType type;
    
	float values[CHUNK_SIZE * CHUNK_SIZE];
    
	Chunk(u8 x, u8 y, Area** areas, ChunkType type);
	void calculate();
	void renderToPGM(std::string& filename);

    private:
    
	void calculate_inner();
	void calculate_vertical();
	void calculate_horizontal();
	void calculate_corner();
};