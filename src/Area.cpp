/**
*  @file    Area.cpp
*  @author  
*  @date    05 June 2018
*/

#include "Area.h"
#include <array>
#include <chrono>
#include <numeric>
#include <random>
#include <algorithm>
#include "TypeDef.h"

Area::Area(u8 x, u8 y) : x(x), y(y)
{
	// https://stackoverflow.com/questions/5008804/generating-random-integer-from-a-range
    std::random_device rd;   // only used once to initialise (seed) engine
    std::mt19937 rng(rd());  // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> random_int(3, 7);
    std::uniform_real_distribution<float> random_float(0.1f, 1.0f);

    amplitude = random_float(rng);
    frequency = random_float(rng);
    octaves = random_int(rng);

	// Perlin Random Initialize
    // C++11 Randoms http://www.cplusplus.com/reference/random/
	u32 seed = (u32)std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    // Fill with 0 to 255
    auto permEnd = permutation.begin() + 256;
    std::iota(permutation.begin(), permEnd, 1);
    // Shuffle
    std::shuffle(permutation.begin(), permEnd, generator);
    // Copy back to avoid overflows (we can query from [0,512[ to avoid doing modulos everywhere
    std::copy(permutation.begin(), permEnd, permEnd);
}
