#pragma once

namespace Turbo
{
   struct Engine;
   // TODO(SS): replace with platform performance counters
	using FChronoTimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	struct CoreTimer
	{
   	FChronoTimePoint mEngineStartTime {};
   	FChronoTimePoint mTickStartTime {};

   	fp32 mDeltaTime = 0;
   	fp64 mTimeFromEngineStart = 0;
   	u64 mTickIndex = 0;

		/* Engine API */
      void Init(Engine* engine);
      void Shutdown(Engine* engine);

      void Tick();
	};
} // Turbo
