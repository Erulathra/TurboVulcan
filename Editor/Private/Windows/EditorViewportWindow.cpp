#include "Windows/EditorViewportWindow.h"

#include "imgui.h"
#include "Core/Engine.h"
#include "EditorViewPort/EditorFreeCamera.h"
#include "Graphics/GPUDevice.h"
#include "Layers/ImGUILayer.h"
#include "Windows/EditorGizmo.h"
#include "World/Camera.h"
#include "World/World.h"

namespace Turbo
{
	void EditorViewportWindow::Init(Engine* engine)
	{
		EditorFreeCameraUtils::Init(engine);

#if 0
		mGizmo = std::make_unique<FEditorGizmo>();
		mGizmo->Init();
#endif
	}

	void EditorViewportWindow::Shutdown(Engine* engine)
	{
		for (THandle<FTexture> texture : mRenderedTextures)
		{
			engine->mGPU->DestroyTexture(texture);
		}
	}

	void EditorViewportWindow::HandleEvent(FEventBase& event)
	{
		EditorFreeCameraUtils::HandleEvent(event, bHasFocus);

#if 0
		if (event.mEventReply != EEventReply::Handled)
		{
			mGizmo->HandleEvent(event);
		}
#endif
	}

	void EditorViewportWindow::Tick(Engine* engine, fp32 deltaTime)
	{
		ImGui::SetNextWindowSizeConstraints(glm::uint2(128), glm::uint2(UINT_MAX));
		ImGui::Begin("Viewport");
		bHasFocus = ImGui::IsWindowHovered();

		// const glm::uint2 newContentSize = currentWindow->SizeFull
		const glm::uint2 newContentSize = ImGui::GetContentRegionAvail();
		if (newContentSize != mEditorViewportSize)
		{
			ResizeViewport(engine, newContentSize);
		}

		const u32 frameInFlight = engine->mGPU->GetFrameInFlightId();
		if (frameInFlight < mRenderedTextures.size())
		{
			ImGui::Texture(engine->mGPU, engine->mImGuiLayer, mRenderedTextures[frameInFlight]);
		}

	#if 0
		mGizmo->Draw();
	#endif

		ImGui::End();
	}

	void EditorViewportWindow::ResizeViewport(Engine* engine, const glm::uint2& newSize)
	{
		mEditorViewportSize = newSize;

		engine->mGPU->WaitIdle();
		engine->mGPU->SetMainViewportSize(newSize);

		World* world = engine->mWorld;
		world->mRegistry.view<FCamera>().each([&](entt::entity entity, FCamera& camera)
		{
			camera.mAspectRatio = static_cast<fp32>(newSize.x) / static_cast<fp32>(newSize.y);
			world->mRegistry.get_or_emplace<FProjectionDirty>(entity);
		});

		// Destroy old textures
		for (THandle<FTexture> texture : mRenderedTextures)
		{
			engine->mGPU->DestroyTexture(texture);
		}
		mRenderedTextures.clear();

		// Create new ones
		const u32 numBufferedFrames = engine->mGPU->GetNumBufferedFrames();
		mRenderedTextures.reserve(numBufferedFrames);
		for (u32 frameId = 0; frameId < numBufferedFrames; ++frameId)
		{
			FTextureBuilder builder = {};
			builder
				.Init(vk::Format::eR8G8B8A8Unorm, ETextureType::Texture2D, ETextureFlags::Default)
				.SetSize(glm::uint3(newSize, 1))
				.SetName(FName(fmt::format("EditorViewport_{}", frameId)));

			mRenderedTextures.push_back(engine->mGPU->CreateTexture(builder));
		}
	}
} // Turbo
