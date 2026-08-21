#include "Core/Math/Random.h"

#include <random>

#include "glm/gtx/color_space.hpp"

namespace Turbo
{
	u32 gSeed = 0;

	void Random::SetRandomSeed()
	{
		std::random_device randomDevice;
		gSeed = randomDevice();
	}

	void Random::SetSeed(u32 seed)
	{
		gSeed = seed;
	}

	u32 Random::RandomInt()
	{
		std::mt19937 mt(gSeed);
		gSeed = mt();

		return gSeed;
	}

	fp32 Random::RandomFloat()
	{
		return static_cast<fp32>(RandomInt()) / static_cast<fp32>(std::numeric_limits<u32>::max());
	}

	i32 Random::RandomRange(i32 min, i32 max)
	{
		TURBO_CHECK(min < max);

		return (static_cast<i32>(RandomInt()) % (max - min)) + min;
	}

	fp32 Random::RandomRange(fp32 min, fp32 max)
	{
		TURBO_CHECK(min < max);

		return RandomFloat() * (max - min) + min;
	}

	glm::float3 Random::RandomColor(fp32 saturation, fp32 value)
	{
		return glm::rgbColor(glm::float3(RandomFloat() * 360.f, saturation, value));
	}
} // Turbo
