#pragma once

#include "Core/Input/Input.h"
#include "Layers/Event.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"

struct SDL_KeyboardEvent;

namespace Turbo
{
   struct Engine;

	struct InputSystem
	{
		entt::dense_map<FName /* actionName */, FActionBinding> mActionBindings;
		std::unordered_map<FName /* keyName */, fp32> mLastAxisValues;
		std::unordered_map<FName /* keyName */, bool> mLastKeyValues;

		/* Layer API */
		void Init(Engine* engine);
		void Shutdown(Engine* engine);

		void HandleEvent(FEventBase& event);

		fp32 GetAxisValue(const FKey& key);
		fp32 GetActionValue(FName actionName);

		bool IsKeyPressed(const FKey& key);
		bool IsActionPressed(FName actionName);

		bool RegisterBinding(const FActionBinding& actionBinding);

		/* Internal */
		void HandleSDLKeyboardEvent(Engine* engine, const SDL_KeyboardEvent& keyboardEvent);
		void HandleSDLMouseButtonEvent(Engine* engine, const SDL_MouseButtonEvent& mouseButtonEvent);
		void HandleSDLMouseMotionEvent(Engine* engine, const SDL_MouseMotionEvent& mouseMotionEvent);
		void HandleSDLMouseWheelEvent(Engine* engine, const SDL_MouseWheelEvent& mouseWheelEvent);

		static FKey ConvertSDLKey(SDL_Keycode key);
		static FKey ConvertSDLMouseButton(u8 mouseButtonIndex);

		void HandleKeyEvent(Engine* engine, FKeyEvent& keyEvent);
		void HandleAxisEvent(Engine* engine, FAxisEvent& axisEvent);
	};
} // Turbo
