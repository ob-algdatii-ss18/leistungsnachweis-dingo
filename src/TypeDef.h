#pragma once
#include <SDL.h>
#include <cstdint>

#define Assert(Expression) SDL_assert(Expression)
#define PI 3.14159265359f
#define DEG_TO_RAD(degrees) (2.0f*(float)PI / 360.0f) * degrees

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define CHUNK_STRIDE 3
#define CHUNK_SIZE 64
#define AREA_STRIDE 2
