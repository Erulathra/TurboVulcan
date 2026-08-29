#pragma once

#include "Graphics/GPUDevice.h"
union SDL_Event;

namespace Turbo
{
   struct FTexture;
   struct FRGResourceHandle;
   struct ImGuiLayer;
   struct RenderGraph;
}

namespace ImGui
{
	using FTextureId = u32;
	void Texture(Turbo::GPUDevice* gpu, Turbo::ImGuiLayer* imguiLayer, Turbo::THandle<Turbo::FTexture> textureHandle);
}

namespace Turbo
{
   struct Engine;
   struct Window;
   struct RenderGraph;

   struct ImGuiTexture
	{
		THandle<FTexture> mTexture;
		FRGResourceHandle mRGTexture = {};
		vk::DescriptorSet mDescriptorSet = {};
	};

	struct ImGuiLayer
	{
	   // TODO(SS): Replace with static sized stack
   	std::vector<ImGuiTexture> mTextures;

      /* Public api */
		ImGuiTexture& FindOrRegisterTexture(THandle<FTexture> textureHandle);

		/** Service Api */
		void Init(Engine* engine);
		void Shutdown(Engine* engine);

		void BeginTick(fp64 deltaTime);
		void EndTick(fp64 deltaTime);

		void BeginPresentingFrame(GPUDevice* gpu, RenderGraph* renderGraph, FRGResourceHandle presentImage);

		/* Internals */
		void OnSDLEvent(Window* window, SDL_Event* sdlEvent);
		void SetupTheme();
	};

} // Turbo
