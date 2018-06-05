/**
*  @file    Chunk.cpp
*  @author  
*  @date    05 June 2018
*/

#include "Chunk.h"
#include "Perlin.h"

Chunk::Chunk(u8 x, u8 y, Area a) : x(x), y(y)
{
    int z = 0;

	// X and Y are our base coordinates.
	// Our chunk goes from X,Y to X+16,Y+16 (CHUNK_SIZE)

    for (int xx = 1; xx <= CHUNK_SIZE; ++xx)
    {
        for (int yy = 1; yy <= CHUNK_SIZE; ++yy)
        {
            values[xx * yy] = octavePerlin(xx + x, yy + y, 0, a);
        }
    }
}