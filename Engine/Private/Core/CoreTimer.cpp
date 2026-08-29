#include "Core/CoreTimer.h"

#include "Core/Engine.h"

namespace Turbo {
	void CoreTimer::Init(Engine* engine)
	{
		mEngineStartTime = std::chrono::steady_clock::now();
		mTickStartTime = {};
	}

	void CoreTimer::Shutdown(Engine* engine)
	{
		// do nothing
	}

	void CoreTimer::Tick()
	{
		if (mTickIndex == 0)
		{
			mDeltaTime = 0;
			mTimeFromEngineStart = 0;

			mTickStartTime = std::chrono::steady_clock::now();
			++mTickIndex;

			return;
		}

		const FChronoTimePoint newTickStartTime = std::chrono::steady_clock::now();

		mDeltaTime = std::chrono::duration<fp32>(newTickStartTime - mTickStartTime).count();
		mTimeFromEngineStart = std::chrono::duration<fp64>(newTickStartTime - mEngineStartTime).count();

		mTickStartTime = newTickStartTime;
		++mTickIndex;
	}
} // Turbo
