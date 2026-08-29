#include "Layers/ConsoleFrontendLayer.h"

#include "Core/Engine.h"
#include "Core/Input/InputSystem.h"
#include "imgui.h"
#include "Core/Input/Input.h"
#include "Core/Input/Keys.h"
#include "Debug/IConsoleManager.h"
#include "misc/cpp/imgui_stdlib.h"

namespace Turbo
{
	const FName kToggleConsoleName = FName("ToggleConsole");

	void ConsoleFrontendLayer::Init(Engine* engine)
	{
	   // TODO(SS): If we get rid of the standard library that would not be necessary
	   new(this) ConsoleFrontendLayer();

		engine->mInputSystem->RegisterBinding({kToggleConsoleName, EKeys::Grave});

		IConsoleManager& consoleManager = IConsoleManager::Get();
		consoleManager.RegisterCommand(FConsoleCommand(
			"history",
			"Shows command history",
			FConsoleCommandDelegate::CreateLambda([this](IConsoleManager& consoleManager, const FArgsVector args)
			{
				std::string message;
				for (i32 commandId = 0; commandId < mConsoleHistory.size(); ++commandId)
				{
					message += fmt::format("\t{}\t{}\n", commandId, mConsoleHistory[commandId]);
				}

				consoleManager.Print(message);
			})
		));
	}

	void ConsoleFrontendLayer::Shutdown(Engine* engine)
	{
		IConsoleManager& consoleManager = entt::locator<IConsoleManager>::value();
		consoleManager.UnregisterCommand("history");
	}

	i32 OnConsoleInputCallback(ImGuiInputTextCallbackData* data)
	{
		ConsoleFrontendLayer* frontend = static_cast<ConsoleFrontendLayer*>(data->UserData);
		IConsoleManager& consoleManager = entt::locator<IConsoleManager>::value();

		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
			{
				const std::vector<std::string_view> candidates =
					consoleManager.FindAutoCompleteCandidates(std::string_view(data->Buf, data->BufTextLen));
				if (candidates.size() == 1)
				{
					const char* begin = &candidates[0].front() + data->BufTextLen;
					const char* end = &candidates[0].back();
					data->InsertChars(data->BufTextLen, begin, end + 1);
				}
				else if (candidates.size() > 1)
				{
					std::string message;
					for (const std::string_view& candidate : candidates)
					{
						message += candidate;
						message += " ";
					}

					consoleManager.Print(message);
				}
				break;
			}
		case ImGuiInputTextFlags_CallbackHistory:
			{
				const std::vector<std::string>& history = frontend->mConsoleHistory;
				if (frontend->mHistoryIndex == history.size())
				{
					frontend->mTempBuffer = std::string_view(data->Buf, data->BufTextLen);
				}

				i32 newHistoryIndex = frontend->mHistoryIndex;
				if (data->EventKey == ImGuiKey_UpArrow)
				{
					newHistoryIndex--;
				}
				else if (data->EventKey == ImGuiKey_DownArrow)
				{
					newHistoryIndex++;
				}

				frontend->mHistoryIndex = Math::Modulo<i32>(newHistoryIndex, history.size() + 1);

				std::string_view historyEntry;
				if (frontend->mHistoryIndex == history.size())
				{
					historyEntry = frontend->mTempBuffer;
				}
				else
				{
					historyEntry = history[frontend->mHistoryIndex];
				}

				if (historyEntry.size() > 0)
				{
					const char* begin = &historyEntry.front();
					const char* end = &historyEntry.back();

					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, begin, end + 1);
				}

				break;
			}
		default: ;
		}

		return 0;
	}

	void ConsoleFrontendLayer::BeginTick(fp64 deltaTime)
	{
		std::string inputBuffer;

		ImGui::Begin("Console");
		ImGui::PushItemWidth(-1);

		ImVec2 regionSize = ImGui::GetContentRegionAvail();
		regionSize.y -= ImGui::GetTextLineHeightWithSpacing() + (2.f * ImGui::GetStyle().FramePadding.y);
		ImGui::InputTextMultiline("##ConsoleOutput", &mConsoleBuffer, regionSize, ImGuiInputTextFlags_ReadOnly);

		constexpr ImGuiInputTextFlags inputTextFlags =
			ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_EnterReturnsTrue;

		if (ImGui::InputText("##ConsoleInput", &inputBuffer, inputTextFlags, OnConsoleInputCallback, this))
		{
			IConsoleManager& consoleManager = entt::locator<IConsoleManager>::value();
			consoleManager.Printf("$ {}", inputBuffer);
			mConsoleHistory.push_back(inputBuffer);
			consoleManager.Parse(inputBuffer);

			mHistoryIndex = mConsoleHistory.size();
			mbFocusConsoleInput = true;
		}
		ImGui::PopItemWidth();

		if (mbFocusConsoleInput)
		{
			ImGui::SetKeyboardFocusHere(-1);
			mbFocusConsoleInput = false;
		}

		ImGui::End();
	}

	void ConsoleFrontendLayer::HandleEvent(FEventBase& event)
	{
		EventDispatcher::Dispatch<FActionEvent>(event, &ConsoleFrontendLayer::HandleInputActionEvent, this);
		EventDispatcher::Dispatch<FConsoleBufferChangedEvent>(event, &ConsoleFrontendLayer::HandleConsoleBufferChangedEvent, this);
	}

	void ConsoleFrontendLayer::HandleInputActionEvent(FActionEvent& event, ConsoleFrontendLayer* layer)
	{
		if (event.mName == kToggleConsoleName && event.mbDown)
		{
			layer->mbConsoleVisible = !layer->mbConsoleVisible;
			if (layer->mbConsoleVisible)
			{
				layer->mbFocusConsoleInput = true;
			}
		}
	}

	void ConsoleFrontendLayer::HandleConsoleBufferChangedEvent(FConsoleBufferChangedEvent& event, ConsoleFrontendLayer* layer)
	{
		layer->mConsoleBuffer += event.mMessage;
		layer->mConsoleBuffer += "\n";
	}
}
