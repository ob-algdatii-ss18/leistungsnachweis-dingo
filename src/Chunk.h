/**
 *  @file    Chunk.h
 *  @author
 *  @date    05 June 2018
 */

#pragma once

#include <vector>
#include "Area.h"
#include "TypeDef.h"

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

struct Chunk
{
    Chunk(u8 x, u8 y, std::vector<Area*> areas, ChunkType type, Area* def);

    float values[CHUNK_SIZE * CHUNK_SIZE];

    void calculate();
    void renderToPGM(std::string const& filename);

    // Beim Rendern als offset benutzen um die absoluten Koordinaten der Werte zu erhalten.
    u32 x;
    u32 y;

    Area* area;

    // 1 inner			1 Area
    // 2 vertical		2 Areas (Oben, Unten)
    // 3 horizontal		2 Areas (Rechts, Links)
    // 4 corner			4 Areas (Oben Links, Oben Rechts, Unten Links, Unten Rechts)
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

inline std::vector<Area> g_areas;
