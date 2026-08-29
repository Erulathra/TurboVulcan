#pragma once

#include "Core/Engine.h"
namespace Turbo
{
	class FEditorGizmo;
	struct FTexture;
	struct FEventBase;
	struct ImGuiLayer;

	struct EditorViewportWindow
	{
	#if 0
   	TUniquePtr<FEditorGizmo> mGizmo;
   #endif

   	std::vector<THandle<FTexture>> mRenderedTextures;
   	glm::uint2 mEditorViewportSize = glm::uint2(0);
   	bool bHasFocus = false;

		void Init(Engine* engine);
		void Shutdown(Engine* engine);
		void Tick(Engine* engine, fp32 deltaTime);
		void ResizeViewport(Engine* engine, const glm::uint2& newSize);

		void HandleEvent(FEventBase& event);
	};
} // Turbo
