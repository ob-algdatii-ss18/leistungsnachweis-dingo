/**
*  @file    Chunk.cpp
*  @author  
*  @date    05 June 2018
*/

#include "Chunk.h"
#include "Perlin.h"

#define Z_VALUE 0

Chunk::Chunk(u8 x, u8 y, Area areas[], u8 type) : x(x * CHUNK_SIZE), y(y * CHUNK_SIZE), type(type)
{
	// Ich bin irgendwie zu dumm das richtig zu machen XD
    this->areas[0] = areas[0];
    this->areas[1] = areas[1];
    this->areas[2] = areas[2];
    this->areas[3] = areas[3];

	if (type < 0 || type > 3)
		throw std::invalid_argument("Illegal chunk type - must be in between 0 and 3");
}

void Chunk::calculate() // Hier könnte man evtl. Z übergeben, falls man später doch wieder animieren will..
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
    for (int xx = 0; xx < CHUNK_SIZE; ++xx)
    {
        for (int yy = 0; yy < CHUNK_SIZE; ++yy)
        {
            values[xx * CHUNK_SIZE + yy] = octavePerlin(xx + x, yy + y, Z_VALUE, areas[0]);
        }
    }
}

void Chunk::calculate_vertical()
{
    Area upper_area = areas[0];
    Area lower_area = areas[1];

    for (int xx = 0; xx < CHUNK_SIZE; ++xx)
    {
        for (int yy = 0; yy < CHUNK_SIZE; ++yy)
        {
            float upper = octavePerlin(xx + x, yy + y, Z_VALUE, upper_area);
            float lower = octavePerlin(xx + x, yy + y, Z_VALUE, lower_area);

            values[xx * CHUNK_SIZE + yy] = 0; // interpolieren ziwschen upper & lower in Abhängigkeit von yy
        }
    }
}

void Chunk::calculate_horizontal()
{
    Area left_area = areas[0];
    Area right_area = areas[1];

    for (int xx = 0; xx < CHUNK_SIZE; ++xx)
    {
        for (int yy = 0; yy < CHUNK_SIZE; ++yy)
        {
            float left = octavePerlin(xx + x, yy + y, Z_VALUE, left_area);
            float right = octavePerlin(xx + x, yy + y, Z_VALUE, right_area);

            values[xx * CHUNK_SIZE + yy] = 0; // TODO interpolieren zwischen left & right in Abhängigkeit von xx
        }
    }
}

void Chunk::calculate_corner()
{
    Area upper_left_area = areas[0];
    Area upper_right_area = areas[1];
    Area lower_right_area = areas[2];
    Area lower_left_area = areas[3];

    // X and Y are our base coordinates.
    // Our chunk goes from X,Y to X+16,Y+16 (CHUNK_SIZE)

    for (int xx = 0; xx < CHUNK_SIZE; ++xx)
    {
        for (int yy = 0; yy < CHUNK_SIZE; ++yy)
        {
            float upper_left = octavePerlin(xx + x, yy + y, Z_VALUE, upper_left_area);
            float upper_right = octavePerlin(xx + x, yy + y, Z_VALUE, upper_right_area);
            float lower_right = octavePerlin(xx + x, yy + y, Z_VALUE, lower_right_area);
            float lower_left = octavePerlin(xx + x, yy + y, Z_VALUE, lower_left_area);

            values[xx * CHUNK_SIZE + yy] = 0;  // TODO zwischen a b c d interpolieren
        }
    }
}