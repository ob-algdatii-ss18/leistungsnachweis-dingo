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

    for (int y = 1; y <= CHUNK_SIZE; ++y)
    {
        for (int x = 1; x <= CHUNK_SIZE; ++x)
        {
            values[y * x] = octavePerlin(x, y, 0, a);
        }
    }
}