#pragma once

namespace Turbo
{
	using FChronoTimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	class FCoreTimer
	{
	private:
		FCoreTimer() = default;

	public:
		void Init();
		void Destroy();

		static FCoreTimer* Get();
		[[nodiscard]] static fp64 DeltaTime() { return Get()->GetDeltaTime(); }
		[[nodiscard]] static fp64 TimeFromEngineStart() { return Get()->GetTimeFromEngineStart(); }
		[[nodiscard]] static u64 TickIndex() { return Get()->GetTickIndex(); }

		DELETE_COPY(FCoreTimer)

	protected:
		void Tick();

	private:
		[[nodiscard]] fp64 GetDeltaTime() const { return mDeltaTime; }
		[[nodiscard]] fp64 GetTimeFromEngineStart() const { return mTimeFromEngineStart; };
		[[nodiscard]] u64 GetTickIndex() const { return mTickIndex; };

	private:
		FChronoTimePoint mEngineStartTime {};
		FChronoTimePoint mTickStartTime {};

		fp64 mDeltaTime = -1.;
		fp64 mTimeFromEngineStart = -1.;
		u64 mTickIndex = 0;

	public:
		friend class Engine;
	};
} // Turbo
