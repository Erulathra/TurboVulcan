#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "Core/Delegate.h"
#include "SDL3/SDL_events.h"

DECLARE_LOG_CATEGORY(LogWindow, Info, Display)

namespace Turbo
{
   struct Engine;
   struct Window;

	// TODO: Replace that with config
	namespace WindowDefaultValues
	{
		static constexpr i32 kSizeX = 1280;
		static constexpr i32 kSizeY = 720;
		static constexpr std::string_view kName = "Turbo Vulkan";
	}

	// TODO(SS): Temporarly add poiter to the engine to those delegates
	DECLARE_MULTICAST_DELEGATE(FSDLEventDelegate, Window*, SDL_Event*);
	DECLARE_DELEGATE(FOnSDLKeyboardEvent, Engine*, const SDL_KeyboardEvent&);
	DECLARE_DELEGATE(FOnSDLMouseButtonEvent, Engine*, const SDL_MouseButtonEvent&);
	DECLARE_DELEGATE(FOnSDLMouseMotionEvent, Engine*, const SDL_MouseMotionEvent&);
	DECLARE_DELEGATE(FOnSDLMouseWheelEvent, Engine*, const SDL_MouseWheelEvent&);

	struct Window
	{
	   // TODO(SS): Move SDL implementation details to source file
		SDL_Window* mSDLWindow = nullptr;
		VkSurfaceKHR mVulkanSurface = nullptr;

		FOnSDLKeyboardEvent OnSDLKeyboardEvent;
		FOnSDLMouseButtonEvent OnSDLMouseButtonEvent;
		FOnSDLMouseMotionEvent OnSDLMouseMotionEvent;
		FOnSDLMouseWheelEvent OnSDLMouseWheelEvent;

		SDL_Surface* mWindowIconSurface = nullptr;

		bool mbFullscreenEnabled = false;

		bool Init(Engine* engine);
		void Shutdown(Engine* engine);

		/** Events */
		FSDLEventDelegate OnSDLEvent;

		/** Basic Interface */
		void PollWindowEventsAndErrors(Engine* engine);

		void ShowWindow(bool bVisible);
		void ShowCursor(bool bVisible);

		[[nodiscard]] glm::uint2 GetFrameBufferSize() const;
		[[nodiscard]] SDL_Window* GetWindow() const { return mSDLWindow; }

		fp32 GetDisplayScale() const;

		[[nodiscard]] bool IsFullscreenEnabled() const;
		void SetFullscreen(bool bFullscreen);

		void SetWindowIcon(std::string_view path);

		/** SDL Interface **/
		void BindKeyboardEvent(const FOnSDLKeyboardEvent& NewDelegate) { OnSDLKeyboardEvent = NewDelegate; }
		void RemoveKeyboardEvent() { OnSDLKeyboardEvent = FOnSDLKeyboardEvent(); }

		void BindMouseButtonEvent(const FOnSDLMouseButtonEvent& NewDelegate) { OnSDLMouseButtonEvent = NewDelegate; }
		void RemoveMouseButtonEvent() { OnSDLMouseButtonEvent = FOnSDLMouseButtonEvent(); }

		void BindMouseMotionEvent(const FOnSDLMouseMotionEvent& NewDelegate) { OnSDLMouseMotionEvent = NewDelegate; }
		void RemoveMouseMotionEvent() { OnSDLMouseMotionEvent = FOnSDLMouseMotionEvent(); }

		void BindMouseWheelEvent(const FOnSDLMouseWheelEvent& NewDelegate) { OnSDLMouseWheelEvent = NewDelegate; }
		void RemoveMouseWheelEvent() { OnSDLMouseWheelEvent = FOnSDLMouseWheelEvent(); }

		[[nodiscard]] std::vector<const char*> GetVulkanRequiredExtensions();

		bool CreateVulkanSurface(VkInstance vulkanInstance);
		[[nodiscard]] VkSurfaceKHR GetVulkanSurface();
		bool DestroyVulkanSurface(VkInstance vulkanInstance);

		/** Internal methods */
		static void LogError();
		static SDL_Surface* LoadSurface(std::string_view path);
	};

} // Turbo
