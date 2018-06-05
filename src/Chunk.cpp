/**
*  @file    Chunk.cpp
*  @author  
*  @date    05 June 2018
*/

#include "Chunk.h"
#include "Perlin.h"

#define Z_VALUE 0

Chunk::Chunk(u8 x, u8 y, Area areas[4], u8 type) : x(x), y(y), type(type)
{
	// Ich bin irgendwie zu dumm das richtig zu machen XD
    this->areas[0] = areas[0];
    this->areas[1] = areas[1];
    this->areas[2] = areas[2];
    this->areas[3] = areas[3];

	if (type < 0 || type > 3)
		throw std::invalid_argument("Illegal chunk type - must be in between 0 and 3");
}

void Chunk::calculate() // Hier k�nnte man evtl. Z �bergeben, falls man sp�ter doch wieder animieren will..
{
	switch (type)
	{
		case 0: calculate_inner(); break;
		case 1: calculate_vertical(); break;
		case 2: calculate_horizontal(); break;
		case 3: calculate_corner(); break;
		default: throw std::invalid_argument("Illegal chunk type - must be in between 0 and 3"); break;
	}
}

void Chunk::calculate_inner()
{
    for (int xx = 1; xx <= CHUNK_SIZE; ++xx)
    {
        for (int yy = 1; yy <= CHUNK_SIZE; ++yy)
        {
            values[xx * yy] = octavePerlin(xx + x, yy + y, Z_VALUE, areas[0]);
        }
    }
}

void Chunk::calculate_vertical()
{
    Area upper_area = areas[0];
    Area lower_area = areas[1];

    for (int xx = 1; xx <= CHUNK_SIZE; ++xx)
    {
        for (int yy = 1; yy <= CHUNK_SIZE; ++yy)
        {
            float upper = octavePerlin(xx + x, yy + y, Z_VALUE, upper_area);
            float lower = octavePerlin(xx + x, yy + y, Z_VALUE, lower_area);

            values[xx * yy] = 0; // interpolieren ziwschen upper & lower in Abh�ngigkeit von yy
        }
    }
}

void Chunk::calculate_horizontal()
{
    Area left_area = areas[0];
    Area right_area = areas[1];

    for (int xx = 1; xx <= CHUNK_SIZE; ++xx)
    {
        for (int yy = 1; yy <= CHUNK_SIZE; ++yy)
        {
            float left = octavePerlin(xx + x, yy + y, Z_VALUE, left_area);
            float right = octavePerlin(xx + x, yy + y, Z_VALUE, right_area);

            values[xx * yy] = 0; // TODO interpolieren zwischen left & right in Abh�ngigkeit von xx
        }
    }
}

void Chunk::calculate_corner()
{
    Area upper_left = areas[0];
    Area upper_right = areas[1];
    Area lower_right = areas[2];
    Area lower_left = areas[3];

    // X and Y are our base coordinates.
    // Our chunk goes from X,Y to X+16,Y+16 (CHUNK_SIZE)

    for (int xx = 1; xx <= CHUNK_SIZE; ++xx)
    {
        for (int yy = 1; yy <= CHUNK_SIZE; ++yy)
        {
            float a = octavePerlin(xx + x, yy + y, Z_VALUE, upper_left);
            float b = octavePerlin(xx + x, yy + y, Z_VALUE, upper_right);
            float c = octavePerlin(xx + x, yy + y, Z_VALUE, lower_right);
            float d = octavePerlin(xx + x, yy + y, Z_VALUE, lower_left);

			values[xx * yy] = 0; // TODO zwischen a b c d interpolieren
        }
    }
}