/**
*  @file    Chunk.cpp
*  @author
*  @date    05 June 2018
*/


#include <fstream>
#include "Chunk.h"
#include "Perlin.h"

#define Z_VALUE 0

Chunk::Chunk(u8 x, u8 y, Area** areas, ChunkType type) : x(x * CHUNK_SIZE), y(y * CHUNK_SIZE), type(type)
{
	// Ich bin irgendwie zu dumm das richtig zu machen XD
	this->areas[0] = areas[0];

	if (type != ChunkType::Inner)
	{
		this->areas[1] = areas[1];
	}
	if (type == ChunkType::Corner)
	{
		this->areas[2] = areas[2];
		this->areas[3] = areas[3];
	}

	if (type < 0 || type > 3)
		throw std::invalid_argument("Illegal chunk type - must be in between 0 and 3");
}

void Chunk::calculate() // Hier könnte man evtl. Z übergeben, falls man später doch wieder animieren will..
{
	switch (type)
	{
	case ChunkType::Inner: calculate_inner(); break;
	case ChunkType::Vertical: calculate_vertical(); break;
	case ChunkType::Horizontal: calculate_horizontal(); break;
	case ChunkType::Corner: calculate_corner(); break; 
	default: throw std::invalid_argument("Illegal chunk type - must be in between 0 and 3"); break;
	}
}

void Chunk::renderToPGM(std::string& filename)
{
	std::ofstream out(filename);
	if (!out)
		return;
	out << "P2" << std::endl;
	out << CHUNK_SIZE << " " << CHUNK_SIZE << std::endl;
	out << "255" << std::endl;
	for (int row = 0; row < CHUNK_SIZE; ++row)
	{
		for (int col = 0; col < CHUNK_SIZE; ++col)
		{
			int value = values[row * CHUNK_SIZE + col] * 255;
			out << value << " ";
		}
		out << std::endl;
	}

	out.close();
}

void Chunk::calculate_inner()
{
	for (int yy = 0; yy < CHUNK_SIZE; ++yy)
	{
		const int row_offset = yy * CHUNK_SIZE;
		for (int xx = 0; xx < CHUNK_SIZE; ++xx)
		{
			values[row_offset + xx] = octavePerlin(xx + x, yy + y, Z_VALUE, *areas[0]);
		}
	}
}

void Chunk::calculate_vertical()
{
	const Area& upper_area = *areas[0];
	const Area& lower_area = *areas[1];

	for (int yy = 0; yy < CHUNK_SIZE; ++yy)
	{
		const int row_offset = yy * CHUNK_SIZE;
		for (int xx = 0; xx < CHUNK_SIZE; ++xx)
		{
			float upper = octavePerlin(xx + x, yy + y, Z_VALUE, upper_area);
			float lower = octavePerlin(xx + x, yy + y, Z_VALUE, lower_area);

			values[row_offset + xx] = 0; // interpolieren ziwschen upper & lower in Abhängigkeit von yy
		}
	}
}

void Chunk::calculate_horizontal()
{
	const Area& left_area = *areas[0];
	const Area& right_area = *areas[1];

	for (int yy = 0; yy < CHUNK_SIZE; ++yy)
	{
		const int row_offset = yy * CHUNK_SIZE;
		for (int xx = 0; xx < CHUNK_SIZE; ++xx)
		{
			float left = octavePerlin(xx + x, yy + y, Z_VALUE, left_area);
			float right = octavePerlin(xx + x, yy + y, Z_VALUE, right_area);

			values[row_offset + xx] = 0; // TODO interpolieren zwischen left & right in Abhängigkeit von xx
		}
	}
}

void Chunk::calculate_corner()
{
	const Area& upper_left_area = *areas[0];
	const Area& upper_right_area = *areas[1];
	const Area& lower_right_area = *areas[2];
	const Area& lower_left_area = *areas[3];

	// X and Y are our base coordinates.
	// Our chunk goes from X,Y to X+16,Y+16 (CHUNK_SIZE)

	for (int yy = 0; yy < CHUNK_SIZE; ++yy)
	{
		const int row_offset = yy * CHUNK_SIZE;
		for (int xx = 0; xx < CHUNK_SIZE; ++xx)
		{
			float upper_left = octavePerlin(xx + x, yy + y, Z_VALUE, upper_left_area);
			float upper_right = octavePerlin(xx + x, yy + y, Z_VALUE, upper_right_area);
			float lower_right = octavePerlin(xx + x, yy + y, Z_VALUE, lower_right_area);
			float lower_left = octavePerlin(xx + x, yy + y, Z_VALUE, lower_left_area);

			values[row_offset + xx] = 0;  // TODO zwischen a b c d interpolieren
		}
	}
}