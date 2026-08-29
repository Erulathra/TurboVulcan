#pragma once

#include "CommonTypeDefs.h"
#include "Core/Allocators/Arena.h"
#include "Graphics/FrameGraph/RenderGraph.h"
#include "Graphics/FrameGraph/RenderGraphHelpers.h"
#include "Layers/Event.h"

namespace Turbo
{
	struct ConsoleFrontendLayer;
	struct CoreTimer;
	struct GPUDevice;
	struct ImGuiLayer;
	struct SceneRenderingLayer;
	struct World;
   struct Engine;
   struct InputSystem;
   struct Window;

	enum class EWindowEvent : u32;

	enum class EExitCode : i32
	{
		Success = 0,
		WindowCreationError,
		RHICriticalError,
		DeviceNotSupported
	};

	struct PlatformMemory
	{
   	static constexpr SizeType kPersistentDataSize = 64 * Memory::kMebi;
      static constexpr SizeType kTransientDataSize = 4 * Memory::kGibi;

		Arena mPersistentData;
		Arena mTransientData;
	};

	struct RuntimeModule
	{
		using Start                = void (*)(void* /* userData */, Engine* /* engine */);
		using Shutdown             = void (*)(void* /* userData */, Engine* /* engine */);
		using Tick                 = void (*)(void* /* userData */, Engine* /* engine */, fp32 /* deltaTime */);
		using CopyPresentTexture   = void (*)(void* /* userData */, Engine* /* engine */, RenderGraph* /* graphBuilder */, FRGResourceHandle /* presentTexture */);
		using HandleEvent          = void (*)(void* /* userData */, Engine* /* engine */, FEventBase& event);

		void* mUserData = nullptr;

		Start mfStart = nullptr;
		Shutdown mfShutdown = nullptr;
		Tick mfTick = nullptr;
		CopyPresentTexture mfCopyPresentTexture = nullptr;
		HandleEvent mfHandleEvent = nullptr;
	};

	struct Engine
	{
	   /* Low-Level stuff */
		PlatformMemory* mPlatformMemory;
		CoreTimer* mCoreTimer;
		Window* mWindow;
		GPUDevice* mGPU;
		RenderGraph* mRenderGraph;
		InputSystem* mInputSystem;

      /* Layers */
      SceneRenderingLayer* mSceneRenderingLayer;
      ImGuiLayer* mImGuiLayer;
      ConsoleFrontendLayer* mDeveloperConsoleLayer;

     	World* mWorld;

      /* Runtime Module API */
      RuntimeModule* mRuntimeModule;

      bool mbExitRequested;
      EExitCode mExitCode;

      /* Start up */
		i32 Start(PlatformMemory* platformMemory);

		/* Shutdown */
		void RequestExit(EExitCode InExitCode = EExitCode::Success);
		void Shutdown();

		/* Working engine */
		void GameThreadLoop();
		EEventReply PushEvent(FEventBase& event);
	};

	extern Engine* InitEngine(PlatformMemory* memory, i32 argc, char* argv[]);
} // namespace Turbo
