/**
 *  @file    Chunk.cpp
 *  @author
 *  @date    05 June 2018
 */

#include "Chunk.h"
#include <fstream>
#include "Perlin.h"
std::vector<Area> g_areas;
#define Z_VALUE 0
static float clamp(float x, float lowerlimit, float upperlimit)
{
    if (x < lowerlimit)
        x = lowerlimit;
    if (x > upperlimit)
        x = upperlimit;
    return x;
}
static float smoothstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    float t = x * x * (3 - 2 * x);
    // Evaluate polynomial
    return (1.f - t) * edge0 + t * edge1;
}

static float linear(float edge0, float edge1, float t)
{
    // Evaluate polynomial
    return (1.f - t) * edge0 + t * edge1;
}
static float interpolate(float x1, float x2, float t)
{
    Assert(t >= 0.f && t <= 1.f);
    return smoothstep(x1, x2, t);
}

Chunk::Chunk(u8 x, u8 y, std::vector<Area*> areas, ChunkType type, Area* def)
    : x(x * CHUNK_SIZE), y(y * CHUNK_SIZE), area(def), type(type)
{
    // Ich bin irgendwie zu dumm das richtig zu machen XD
    this->areas[0] = areas[0];

    if (type != ChunkType::Inner)
    {
        this->areas[1] = areas[1];
    }
    if (type > 4)
    {
        this->areas[2] = areas[2];
        this->areas[3] = areas[3];
    }

    if (type < 0 || type >= Length)
        throw std::invalid_argument("Illegal chunk type - must be in between 0 and 3");
}

#define INTERP 48
void Chunk::calculate()  // Hier könnte man evtl. Z übergeben, falls man später doch wieder
                         // animieren will..
{
    switch (type)
    {
        case ChunkType::Inner:
            calculate_inner();
            break;
        case ChunkType::VerticalTop:
            calculate_vertical_top();
            break;
        case ChunkType::VerticalBottom:
            calculate_vertical_bottom();
            break;
        case ChunkType::HorizontalLeft:
            calculate_horizontal_left();
            break;
        case ChunkType::HorizontalRight:
            calculate_horizontal_right();
            break;
        case ChunkType::CornerTopLeft:
            calculate_corner_tl();
            break;
        case ChunkType::CornerTopRight:
            calculate_corner_tr();
            break;
        case ChunkType::CornerBottomLeft:
            calculate_corner_bl();
            break;
        case ChunkType::CornerBottomRight:
            calculate_corner_br();
            break;
        default:
            throw std::invalid_argument("Illegal chunk type - must be in between 0 and 3");
            break;
    }
    was_calculated = true;
}

void Chunk::renderToPGM(std::string const& filename)
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

void Chunk::calculate_vertical_bottom()
{
    const Area& upper_area = *areas[0];
    const Area& lower_area = *areas[1];

    for (int yy = 0; yy < CHUNK_SIZE; ++yy)
    {
        const int row_offset = yy * CHUNK_SIZE;
        for (int xx = 0; xx < CHUNK_SIZE; ++xx)
        {
            float ty = (float)(yy - INTERP) / (float)(CHUNK_SIZE - INTERP - 1);
            ty = clamp(ty, 0.f, 1.f);

            float z1 = octavePerlin(xx + x, yy + y, Z_VALUE, upper_area);
            float z2 = octavePerlin(xx + x, 0, Z_VALUE, lower_area);
            values[row_offset + xx] = interpolate(z1, z2, ty);
        }
    }
}

void Chunk::calculate_vertical_top()
{
    const Area& upper_area = *areas[0];
    const Area& lower_area = *areas[1];

    for (int yy = 0; yy < CHUNK_SIZE; ++yy)
    {
        const int row_offset = yy * CHUNK_SIZE;
        for (int xx = 0; xx < CHUNK_SIZE; ++xx)
        {
            float ty = (float)(((CHUNK_SIZE - INTERP - 1) - yy) - INTERP) /
                       (float)(CHUNK_SIZE - INTERP - 1);
            ty = clamp(ty, 0.f, 1.f);

            float z2 = octavePerlin(xx + x, (CHUNK_SIZE - 1) + y, Z_VALUE, upper_area);
            float z1 = octavePerlin(xx + x, yy + y, Z_VALUE, lower_area);
            values[row_offset + xx] = interpolate(z1, z2, ty);
        }
    }
}

void Chunk::calculate_horizontal_right()
{
    const Area& left_area = *areas[0];
    const Area& right_area = *areas[1];

    for (int yy = 0; yy < CHUNK_SIZE; ++yy)
    {
        const int row_offset = yy * CHUNK_SIZE;
        for (int xx = 0; xx < CHUNK_SIZE; ++xx)
        {
            float tx = (float)(xx - INTERP) / (float)(CHUNK_SIZE - INTERP - 1);
            tx = clamp(tx, 0.f, 1.f);

            float z1 = octavePerlin(xx + x, yy + y, Z_VALUE, left_area);
            float z2 = octavePerlin(0, yy + y, Z_VALUE, right_area);

            values[row_offset + xx] = interpolate(z1, z2, tx);
        }
    }
}

void Chunk::calculate_horizontal_left()
{
    const Area& left_area = *areas[0];
    const Area& right_area = *areas[1];

    for (int yy = 0; yy < CHUNK_SIZE; ++yy)
    {
        const int row_offset = yy * CHUNK_SIZE;
        for (int xx = 0; xx < CHUNK_SIZE; ++xx)
        {
            float tx = (float)(((CHUNK_SIZE - INTERP - 1) - xx) - INTERP) /
                       (float)(CHUNK_SIZE - INTERP - 1);
            tx = clamp(tx, 0.f, 1.f);

            float z2 = octavePerlin((CHUNK_SIZE - 1) + x, yy + y, Z_VALUE, left_area);
            float z1 = octavePerlin(xx + x, yy + y, Z_VALUE, right_area);

            values[row_offset + xx] = interpolate(z1, z2, tx);
        }
    }
}

void Chunk::calculate_corner_br()
{
    const Area& upper_left_area = *areas[0];
    const Area& upper_right_area = *areas[1];
    const Area& lower_left_area = *areas[2];
    const Area& lower_right_area = *areas[3];

    for (int yy = 0; yy < CHUNK_SIZE; ++yy)
    {
        const int row_offset = yy * CHUNK_SIZE;
        for (int xx = 0; xx < CHUNK_SIZE; ++xx)
        {
            float tx = (float)(xx - INTERP) / (float)(CHUNK_SIZE - INTERP - 1);
            tx = clamp(tx, 0.f, 1.f);

            float ty = (float)(yy - INTERP) / (float)(CHUNK_SIZE - INTERP - 1);
            ty = clamp(ty, 0.f, 1.f);

            // Check which chunk we are

            float z1 = octavePerlin(xx + x, yy + y, Z_VALUE, upper_left_area);
            float z2 = octavePerlin(0, yy + y, Z_VALUE, upper_right_area);
            float z3 = octavePerlin(xx + x, 0, Z_VALUE, lower_left_area);
            float z4 = octavePerlin(0, 0, Z_VALUE, lower_right_area);

            float x1 = interpolate(z1, z2, tx);
            float x2 = interpolate(z3, z4, tx);
            float y = interpolate(x1, x2, ty);

            values[row_offset + xx] = y;
        }
    }
}

void Chunk::calculate_corner_bl()
{
    const Area& upper_left_area = *areas[0];
    const Area& upper_right_area = *areas[1];
    const Area& lower_left_area = *areas[2];
    const Area& lower_right_area = *areas[3];

    for (int yy = 0; yy < CHUNK_SIZE; ++yy)
    {
        const int row_offset = yy * CHUNK_SIZE;
        for (int xx = 0; xx < CHUNK_SIZE; ++xx)
        {
            float tx = (float)(((CHUNK_SIZE - INTERP - 1) - xx) - INTERP) /
                       (float)(CHUNK_SIZE - INTERP - 1);
            tx = clamp(tx, 0.f, 1.f);

            float ty = (float)(yy - INTERP) / (float)(CHUNK_SIZE - INTERP - 1);
            ty = clamp(ty, 0.f, 1.f);

            float z2 = octavePerlin((CHUNK_SIZE - 1) + x, yy + y, Z_VALUE, upper_left_area);
            float z1 = octavePerlin(xx + x, yy + y, Z_VALUE, upper_right_area);
            float z4 = octavePerlin((CHUNK_SIZE - 1) + x, 0, Z_VALUE, lower_left_area);
            float z3 = octavePerlin(xx + x, 0, Z_VALUE, lower_right_area);

            float x1 = interpolate(z1, z2, tx);
            float x2 = interpolate(z3, z4, tx);
            float y = interpolate(x1, x2, ty);

            values[row_offset + xx] = y;
        }
    }
}

void Chunk::calculate_corner_tr()
{
    const Area& upper_left_area = *areas[0];
    const Area& upper_right_area = *areas[1];
    const Area& lower_left_area = *areas[2];
    const Area& lower_right_area = *areas[3];

    for (int yy = 0; yy < CHUNK_SIZE; ++yy)
    {
        const int row_offset = yy * CHUNK_SIZE;
        for (int xx = 0; xx < CHUNK_SIZE; ++xx)
        {
            float tx = (float)(xx - INTERP) / (float)(CHUNK_SIZE - INTERP - 1);
            tx = clamp(tx, 0.f, 1.f);

            float ty = (float)(((CHUNK_SIZE - INTERP - 1) - yy) - INTERP) /
                       (float)(CHUNK_SIZE - INTERP - 1);
            ty = clamp(ty, 0.f, 1.f);

            float z3 = octavePerlin(xx + x, (CHUNK_SIZE - 1) + y, Z_VALUE, upper_left_area);
            float z4 = octavePerlin(0, (CHUNK_SIZE - 1) + y, Z_VALUE, upper_right_area);
            float z2 = octavePerlin(0, yy + y, Z_VALUE, lower_right_area);
            float z1 = octavePerlin(xx + x, yy + y, Z_VALUE, lower_left_area);

            float x1 = interpolate(z1, z2, tx);
            float x2 = interpolate(z3, z4, tx);
            float y = interpolate(x1, x2, ty);

            values[row_offset + xx] = y;
        }
    }
}

void Chunk::calculate_corner_tl()
{
    const Area& upper_left_area = *areas[0];
    const Area& upper_right_area = *areas[1];
    const Area& lower_left_area = *areas[2];
    const Area& lower_right_area = *areas[3];

    for (int yy = 0; yy < CHUNK_SIZE; ++yy)
    {
        const int row_offset = yy * CHUNK_SIZE;
        for (int xx = 0; xx < CHUNK_SIZE; ++xx)
        {
            float tx = (float)(((CHUNK_SIZE - INTERP - 1) - xx) - INTERP) /
                       (float)(CHUNK_SIZE - INTERP - 1);
            tx = clamp(tx, 0.f, 1.f);

            float ty = (float)(((CHUNK_SIZE - INTERP - 1) - yy) - INTERP) /
                       (float)(CHUNK_SIZE - INTERP - 1);
            ty = clamp(ty, 0.f, 1.f);

            float z4 =
                octavePerlin((CHUNK_SIZE - 1) + x, (CHUNK_SIZE - 1) + y, Z_VALUE, upper_left_area);
            float z3 = octavePerlin(xx + x, (CHUNK_SIZE - 1) + y, Z_VALUE, upper_right_area);
            float z1 = octavePerlin(xx + x, yy + y, Z_VALUE, lower_right_area);
            float z2 = octavePerlin((CHUNK_SIZE - 1) + x, yy + y, Z_VALUE, lower_left_area);

            float x1 = interpolate(z1, z2, tx);
            float x2 = interpolate(z3, z4, tx);
            float y = interpolate(x1, x2, ty);

            values[row_offset + xx] = y;
        }
    }
}
