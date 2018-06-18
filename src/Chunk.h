/**
 *  @file    Chunk.h
 *  @author
 *  @date    05 June 2018
 */

#pragma once

#include <vector>
#include "Area.h"
#include "TypeDef.h"


/** Values that represent different chunk types depending on how they are positioned to Areas. */
enum ChunkType : u8
{
    Inner = 0,
    VerticalTop,
    VerticalBottom,
    HorizontalLeft,
    HorizontalRight,
    CornerTopLeft,
    CornerTopRight,
    CornerBottomLeft,
    CornerBottomRight,
    Length
};

/**
 * A chunk holds the values that will be rendered into our 3D model.
 * Chunks live within Areas and use their randomly generated parameters to calculate the perlin noise values.
 *
 * \author VM Ware
 * \date 19.06.2018
 */

struct Chunk
{
    Chunk(u8 x, u8 y, std::vector<Area*> areas, ChunkType type, Area* def);

    float values[CHUNK_SIZE * CHUNK_SIZE];

    void calculate();
    void renderToPGM(std::string const& filename);

    u32 x;
    u32 y;

    Area* area;
    ChunkType type;

   private:
    bool was_calculated = false;
    Area* areas[4];
    void calculate_inner();
    void calculate_vertical_top();
    void calculate_vertical_bottom();
    void calculate_horizontal_left();
    void calculate_horizontal_right();
    void calculate_corner_tl();
    void calculate_corner_tr();
    void calculate_corner_bl();
    void calculate_corner_br();
};

extern std::vector<Area> g_areas;
