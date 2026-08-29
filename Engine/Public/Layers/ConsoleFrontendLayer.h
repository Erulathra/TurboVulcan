#pragma once

#include "Debug/IConsoleManager.h"

class ImGuiInputTextCallbackData;

namespace Turbo
{
   struct Engine;
	struct FConsoleBufferChangedEvent;
	struct FActionEvent;

	struct ConsoleFrontendLayer
	{
	private:
		std::string mConsoleBuffer;
		std::vector<std::string> mConsoleHistory;
		// NOTE(SS): Used when accessing history, as 0th entry.
		std::string mTempBuffer;

		u32 mHistoryIndex = 0;

		bool mbConsoleVisible : 1 = false;
		bool mbFocusConsoleInput : 1 = false;

	public:
		void Init(Engine* engine);
		void Shutdown(Engine* engine);

		void HandleEvent(FEventBase& event);

		void BeginTick(fp64 deltaTime);

	private:
		static void HandleInputActionEvent(FActionEvent& event, ConsoleFrontendLayer* layer);
		static void HandleConsoleBufferChangedEvent(FConsoleBufferChangedEvent& event, ConsoleFrontendLayer* layer);


	public:
		friend i32 OnConsoleInputCallback(ImGuiInputTextCallbackData* data);
	};
}
