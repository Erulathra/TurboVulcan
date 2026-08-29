#include "Editor.h"

#include "Core/Engine.h"
#include "Core/Input/InputSystem.h"
#include "Core/Window.h"
#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Core/Input/Keys.h"
#include "Graphics/Debug.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/FrameGraph/RenderGraphUtils.h"
#include "Layers/Event.h"
#include "Layers/ImGUILayer.h"
#include "Windows/EditorViewportWindow.h"
#include "EditorViewPort/EditorFreeCamera.h"

namespace Turbo
{
	const FName kToggleFullscreenName = FName("ToggleFullscreen");
	const FName kFrameCapture = FName("FrameCapture");
	const FName kRecompileShaders = FName("RecompileShaders");

	void Editor::Start(Engine* engine)
	{
		engine->mInputSystem->RegisterBinding({kRecompileShaders, EKeys::F10});
		engine->mInputSystem->RegisterBinding({kToggleFullscreenName, EKeys::F11});
		engine->mInputSystem->RegisterBinding({kFrameCapture, EKeys::F12});

		mViewportWindow = (EditorViewportWindow*)engine->mPlatformMemory->mPersistentData.Allocate(sizeof(EditorViewportWindow));
		mViewportWindow->Init(engine);
#if ENABLE_FULL_EDITOR
		mOutlinerWindow = MakeShared<FSceneOutlinerWindow>();
		mPropertyEditor = MakeShared<FPropertyEditorWindow>();
		mPropertyEditor->Init();
#endif
	}

	void Editor::Shutdown(Engine* engine)
	{
		mViewportWindow->Shutdown(engine);
	}

	void Editor::Tick(Engine* engine, fp32 deltaTime)
	{
   	EditorFreeCameraUtils::Tick(engine->mWorld, deltaTime);

		mViewportWindow->Tick(engine, deltaTime);
#if ENABLE_FULL_EDITOR
		// mOutlinerWindow->Draw();
		// mPropertyEditor->Draw();
#endif
	}

	void Editor::CopyPresentTexture(Engine* engine, RenderGraph* renderGraph, FRGResourceHandle presentTexture)
	{
		const u32 bufferedFrameId = engine->mGPU->GetFrameInFlightId();

		std::vector<THandle<FTexture>>& renderedTextures = mViewportWindow->mRenderedTextures;
		if (bufferedFrameId < renderedTextures.size())
		{
			const FRGResourceHandle viewportTexture = renderGraph->RegisterExternalTexture(renderedTextures[bufferedFrameId], ETextureLayout::Undefined);
			const FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();
			RenderGraphUtils::AddBlitTexturePass(renderGraph, geometryBuffer.mAfterToneMap, viewportTexture);
		}
	}

	void FEditorSelection::SetSelection(entt::entity entity)
	{
		if (mSelection != entity)
		{
			mSelection = entity;
			OnSelectionChanged.Broadcast(entity);
		}
	}

	void HandleInputActionEvent(FActionEvent& event, Editor* editor)
	{
		if (event.mName == kToggleFullscreenName && event.mbDown)
		{
		   Window* window = event.mEngine->mWindow;
			window->SetFullscreen(!window->IsFullscreenEnabled());
			event.Handle();
		}
		else if (event.mName == kFrameCapture && event.mbDown)
		{
			entt::locator<IFrameDebuggerAPI>::value().CaptureFrame();
			event.Handle();
		}
		else if (event.mName == kRecompileShaders && event.mbDown)
		{
			event.mEngine->mGPU->RecompileShaders();
			event.Handle();
		}
	}

	void HandleCloseEvent(FCloseWindowEvent& event, Editor* editor)
	{
		event.mEngine->RequestExit(EExitCode::Success);
		event.Handle();
	}

	void Editor::HandleEvent(FEventBase& event)
	{
		EventDispatcher::Dispatch<FActionEvent>(event, &HandleInputActionEvent, this);
		EventDispatcher::Dispatch<FCloseWindowEvent>(event, &HandleCloseEvent, this);

		mViewportWindow->HandleEvent(event);
	}
} // Turbo
