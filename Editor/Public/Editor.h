#pragma once

#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Graphics/FrameGraph/RenderGraph.h"

namespace Turbo
{
	DECLARE_MULTICAST_DELEGATE(FOnSelectionChanged, entt::entity);

	class Engine;

	class EditorViewportWindow;
	class FPropertyEditorWindow;
	class FSceneOutlinerWindow;
	class RenderGraph;


	// TODO(SS): Fix editors
	#define ENABLE_FULL_EDITOR 0

	class FEditorSelection
	{
	public:
		FOnSelectionChanged OnSelectionChanged;

	public:
		void SetSelection(entt::entity entity);
		[[nodiscard]] entt::entity GetSelection() const { return mSelection; }

	private:
		entt::entity mSelection = entt::null;
	};

	struct Editor
	{
   	EditorViewportWindow* mViewportWindow;
#if ENABLE_FULL_EDITOR
   	FSceneOutlinerWindow* mOutlinerWindow;
   	FPropertyEditorWindow* mPropertyEditor;
#endif

		void Start(Engine* engine);
		void Shutdown(Engine* engine);
		void Tick(Engine* engine, fp32 deltaTime);
		void CopyPresentTexture(Engine* engine, RenderGraph* graphBuilder, FRGResourceHandle presentTexture);
		void HandleEvent(FEventBase& event);
	};
} // Turbo
