#pragma once

#include "Input/Input.h"
#include "Window.h"

namespace Turbo
{
	class FWorld;
	class FAssetManager;
	class FGPUDevice;
	class FCoreTimer;
	class FVulkanRHI;
	class CommandLineArgsParser;

	enum class EWindowEvent : u32;

	enum class EExitCode : i32
	{
		Success = 0,
		WindowCreationError,
		RHICriticalError,
		DeviceNotSupported
	};

	enum class EEngineState : i32
	{
		Undefined = 0,
		Initializing,
		Running,
		Finalizing
	};

	struct FEngine
	{
		explicit FEngine();

		/** Services */
	public:
		[[nodiscard]] FWorld* GetWorld() const { return mWorld.get(); }

	private:
		TSharedPtr<FWorld> mWorld;

		/** Services end */

	public:
		~FEngine();

	public:
		static FEngine* Init(i32 argc, char* argv[]);
		void RegisterEngineLayers();

		i32 Start();
		void End();

		void RequestExit(EExitCode InExitCode = EExitCode::Success);

	public:
		[[nodiscard]] EEngineState GetEngineState() { return mEngineState; }

	public:
		EEventReply PushEvent(FEventBase& event);

	private:
		void GameThreadLoop();
		void GameThreadTick();

		void OnEvent(FEventBase& event);

	private:
		bool mbExitRequested = false;
		EExitCode mExitCode = EExitCode::Success;

		EEngineState mEngineState = EEngineState::Undefined;
	};

	inline TUniquePtr<FEngine> gEngine;
} // namespace Turbo
