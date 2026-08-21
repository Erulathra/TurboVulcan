#pragma once

namespace Turbo
{
	namespace Random
	{
		void SetRandomSeed();
		void SetSeed(u32 seed);

		u32 RandomInt();
		fp32 RandomFloat();

		i32 RandomRange(i32 min, i32 max);
		fp32 RandomRange(fp32 min = 0.f, fp32 max = 1.f);

		glm::float3 RandomColor(fp32 saturation = 1.f, fp32 value = 1.f);
	} // Random
} // Turbo
