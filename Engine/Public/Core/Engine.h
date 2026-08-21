#pragma once

#include "Input/Input.h"
#include "Window.h"

namespace Turbo
{
	class World;
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

	struct Engine
	{
   	World* mWorld;

      bool mbExitRequested;
      EExitCode mExitCode;

      EEngineState mEngineState;

      /* Start up */
		void RegisterEngineLayers();
		i32 Start();

		/* Shutdown */
		void RequestExit(EExitCode InExitCode = EExitCode::Success);
		void End();

		/* Working engine */
		void GameThreadLoop();
		EEventReply PushEvent(FEventBase& event);

		/* EventHandling */
		// NOTE(SS): That probably we need to refactor
		void OnEvent(FEventBase& event);
	};

	extern void InitEngine(i32 argc, char* argv[]);
} // namespace Turbo
