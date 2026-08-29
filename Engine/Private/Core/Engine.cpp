#include "Core/Engine.h"

#include "Assets/AssetManager.h"
#include "Assets/EngineResources.h"
#include "Assets/MaterialManager.h"
#include "CommonMacros.h"
#include "Core/CommandLineArgs.h"
#include "Core/CoreTimer.h"
#include "Core/FileSystem.h"
#include "Core/Input/InputSystem.h"
#include "Core/Platform.h"
#include "Core/Window.h"
#include "Core/WindowEvents.h"
#include "Debug/IConsoleManager.h"
#include "Graphics/Debug.h"
#include "Graphics/FrameGraph/RenderGraph.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/GeometryBuffer.h"
#include "Layers/ConsoleFrontendLayer.h"
#include "Layers/Event.h"
#include "Layers/ImGUILayer.h"
#include "Layers/SceneRenderingLayer.h"
#include "TurboLog.h"
#include "World/World.h"
#include "entt/locator/locator.hpp"

namespace Turbo
{
	TAutoConsoleVariable<fp32> CVarResolutionScale(
		"r.resolutionScale",
		1.f,
		"The gBuffer resolution scale. This factor multiplies viewport resolution."
	);

	Engine* InitEngine(PlatformMemory* memory, i32 argc, char* argv[])
	{
   	FileSystem::InitDirectories();
   	InitLogger();

   	TURBO_LOG(LogEngine, Info, "Parsing commandline arguments.")
   	FCommandLineArgs::Parse(argc, argv);

   	TURBO_LOG(LogEngine, Info, "Creating engine instance.")

     	Engine* engine = (Engine*)memory->mPersistentData.Allocate(sizeof(Engine));

   #if TURBO_BUILD_SHIPPING == false
   	const static bool bWaitForDebugger = FCommandLineArgs::HasFlag("WaitForAttach");
   	if (bWaitForDebugger)
   	{
   		TURBO_LOG(LogEngine, Info, "Waiting for debugger.")
   		while (FPlatform::IsDebuggerPresent() == false)
   		{
   			FPlatform::Sleep(0.1f);
   		}

   		TURBO_LOG(LogEngine, Info, "Debugger Attached")
   		TURBO_DEBUG_BREAK();
   	}
   #endif // TURBO_BUILD_SHIPPING == false

      return engine;
	}

	i32 Engine::Start(PlatformMemory* platformMemory)
	{
      mPlatformMemory = platformMemory;

		mCoreTimer = (CoreTimer*)platformMemory->mPersistentData.Allocate(sizeof(CoreTimer));
		mCoreTimer->Init(this);

		mWindow = (Window*)platformMemory->mPersistentData.Allocate(sizeof(Window));
		mWindow->Init(this);

		mGPU = (GPUDevice*)platformMemory->mPersistentData.Allocate(sizeof(GPUDevice));
		mGPU->Init(this);

	   // TODO(SS): Frame debugger abstraction
		IFrameDebuggerAPI::Emplace();

		mRenderGraph = (RenderGraph*)platformMemory->mPersistentData.Allocate(sizeof(RenderGraph));
		mRenderGraph->Init(mGPU);

		// TODO(SS): ged rid of the entt
		FAssetManager& assetManager = entt::locator<FAssetManager>::emplace<FAssetManager>();
		assetManager.Init(mGPU);

		FMaterialManager& materialManager = entt::locator<FMaterialManager>::emplace<FMaterialManager>();
		materialManager.Init(mGPU);

		EngineMaterials::InitEngineMaterials(mGPU);

		entt::locator<FGeometryBuffer>::emplace();

		mInputSystem = (InputSystem*)platformMemory->mPersistentData.Allocate(sizeof(InputSystem));
		mInputSystem->Init(this);

		/* Services initialization */
		mSceneRenderingLayer = (SceneRenderingLayer*)platformMemory->mPersistentData.Allocate(sizeof(SceneRenderingLayer));
		mSceneRenderingLayer->Init(this);

		mImGuiLayer = (ImGuiLayer*)platformMemory->mPersistentData.Allocate(sizeof(ImGuiLayer));
		mImGuiLayer->Init(this);

		mDeveloperConsoleLayer = (ConsoleFrontendLayer*)platformMemory->mPersistentData.Allocate(sizeof(ConsoleFrontendLayer));
		mDeveloperConsoleLayer->Init(this);

		// TODO(SS): We need to define arena for world
		mWorld = (World*)platformMemory->mPersistentData.Allocate(sizeof(World));
		mWorld->Init(this);

		SceneGraph::InitSceneGraph(mWorld->mRegistry);

		if (mRuntimeModule && mRuntimeModule->mfStart)
		{
         mRuntimeModule->mfStart(mRuntimeModule->mUserData, this);
		}

		mWindow->SetWindowIcon("Content/Textures/Icons/T_TurboVulkan.png");
		mWindow->ShowWindow(true);

		GameThreadLoop();

		Shutdown();

		return static_cast<i32>(mExitCode);
	}

	EEventReply Engine::PushEvent(FEventBase& event)
	{
      event.mEngine = this;

	   #define CALL_HANDLER(BODY) (BODY); if (event.mEventReply == EEventReply::Handled) { return EEventReply::Handled; }

		CALL_HANDLER(mGPU->HandleEvent(event))
		CALL_HANDLER(mDeveloperConsoleLayer->HandleEvent(event))

		if (mRuntimeModule && mRuntimeModule->mfHandleEvent)
		{
   		CALL_HANDLER(mRuntimeModule->mfHandleEvent(mRuntimeModule->mUserData, this, event))
		}

		#undef CALL_HANDLER

		return event.mEventReply;
	}

	// TODO(SS): Rename service methods to be more verbosive
	void Engine::GameThreadLoop()
	{
		while (!mbExitRequested)
		{
			TRACE_ZONE_SCOPED_N("GameThreadTick")

			mCoreTimer->Tick();
			const fp32 deltaTime = mCoreTimer->mDeltaTime;

			/* Begin tick */
			mImGuiLayer->BeginTick(deltaTime);
			mDeveloperConsoleLayer->BeginTick(deltaTime);

			if (mRuntimeModule && mRuntimeModule->mfTick)
			{
            mRuntimeModule->mfTick(mRuntimeModule->mUserData, this, deltaTime);
			}

			/* End tick */
			mImGuiLayer->EndTick(deltaTime);

			/* Rendering frame */

			if (mGPU->BeginFrame(this))
			{
				FCommandBuffer& cmd = mGPU->GetMainCommandBuffer();
				mRenderGraph->Reset();

				FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();

				TURBO_CHECK(mGPU->GetMainViewportSize() != glm::uint2(0))
				const glm::int2 gbufferResolution
					= glm::floor(glm::float2(mGPU->GetMainViewportSize()) * CVarResolutionScale.Get());
				geometryBuffer.Init(mRenderGraph, gbufferResolution);

				const THandle<FTexture> presentHandle = mGPU->GetPresentImage();
				FRGResourceHandle presentTexture = mRenderGraph->RegisterExternalTexture(
					presentHandle, ETextureLayout::Undefined, ETextureLayout::PresentSrc
				);

				/* Rendering services */
				mSceneRenderingLayer->Render(this);

				if (mRuntimeModule && mRuntimeModule->mfCopyPresentTexture)
				{
   				mRuntimeModule->mfCopyPresentTexture(mRuntimeModule->mUserData, this, mRenderGraph, presentTexture);
				}

				/* Begin presenting frame */
				mImGuiLayer->BeginPresentingFrame(mGPU, mRenderGraph, presentTexture);

				/* Graph builder */
				mRenderGraph->Compile();
				mRenderGraph->Execute(mGPU, cmd);

				/* Present frame */
				mGPU->PresentFrame();
			}

			TRACE_MARK_FRAME();
			mWindow->PollWindowEventsAndErrors(this);
		}
	}

	static void HandleResizeEvent(const FResizeWindowEvent& resizeWindowEvent, Engine* engine)
	{
   	TURBO_LOG(LogEngine, Info, "Window resized. New size {}", resizeWindowEvent.mNewWindowSize)
   	engine->mGPU->RequestSwapChainResize();
	}

	void Engine::Shutdown()
	{
		TURBO_LOG(LogEngine, Info, "Begin exit sequence.");

		mGPU->WaitIdle();
		mGPU->FlushDestroyQueues();

		if (mRuntimeModule && mRuntimeModule->mfShutdown)
		{
         mRuntimeModule->mfShutdown(mRuntimeModule->mUserData, this);
		}

		mWorld->UnloadLevel();

		/* Shutdown services */
		mDeveloperConsoleLayer->Shutdown(this);
		mImGuiLayer->Shutdown(this);
		mSceneRenderingLayer->Shutdown(this);

		EngineResources::DestroyEngineResources(mGPU);

		entt::locator<FAssetManager>::value().Destroy(mGPU);
		entt::locator<FMaterialManager>::value().Destroy(mGPU);

		mRenderGraph->Shutdown(mGPU);
		entt::locator<IFrameDebuggerAPI>::value().Shutdown();

		mGPU->Shutdown(this);

		mInputSystem->Shutdown(this);

		mWindow->Shutdown(this);
		mCoreTimer->Shutdown(this);
	}

	void Engine::RequestExit(EExitCode InExitCode)
	{
		mbExitRequested = true;
		mExitCode = InExitCode;
	}
} // Turbo
