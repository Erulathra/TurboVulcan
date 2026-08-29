#include "Core/Window.h"

#include "backends/imgui_impl_sdl3.h"
#include "Core/Engine.h"
#include "Core/FileSystem.h"
#include "Core/WindowEvents.h"
#include "Core/Input/InputSystem.h"

#include "STB/stb_image.h"

namespace Turbo
{
	bool Window::Init(Engine* engine)
	{
	   // TODO(SS): Replace with ZII
	   new (this) Window();

   	TURBO_LOG(LogWindow, Info, "Initializing SDL.");
   	if (!SDL_Init(SDL_INIT_VIDEO))
   	{
   		LogError();
   	}

      if (!SDL_Vulkan_LoadLibrary(nullptr))
		{
			LogError();
		}

		TURBO_LOG(LogWindow, Info, "Initializing Window.");
		mSDLWindow = SDL_CreateWindow(
			WindowDefaultValues::kName.data(),
			WindowDefaultValues::kSizeX,
			WindowDefaultValues::kSizeY,
			SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE
		);

		if (!mSDLWindow)
		{
			TURBO_LOG(LogWindow, Error, "SDL window creation error. See bellow logs for details");
			LogError();

			return false;
		}

		return true;
	}

	void Window::Shutdown(Engine* engine)
	{
		TURBO_LOG(LogWindow, Info, "Destroying window.");
		SDL_DestroyWindow(mSDLWindow);

		SDL_Vulkan_UnloadLibrary();

		TURBO_LOG(LogWindow, Info, "Stopping SDL.");
		SDL_Quit();
	}


	void Window::LogError()
	{
		TURBO_LOG(LogWindow, Error, "SDL_ERROR: {}", SDL_GetError());
	}

	SDL_Surface* Window::LoadSurface(std::string_view path)
	{
		std::vector<ByteType> imgData;
		if (FileSystem::LoadData(path, imgData) == false)
		{
			return nullptr;
		}

		i32 sizeX, sizeY, numComponents;
		void* pixels = stbi_load(
			path.data(),
			&sizeX,
			&sizeY,
			&numComponents,
			0
		);

		if (pixels == nullptr)
		{
			return nullptr;
		}

		SDL_Surface* result = SDL_CreateSurfaceFrom(
			sizeX,
			sizeY,
			numComponents == 3 ? SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_RGBA32,
			pixels,
			sizeX * numComponents
		);

		if (result == nullptr)
		{
			return nullptr;
		}

		return result;
	}

	void Window::PollWindowEventsAndErrors(Engine* engine)
	{
		SDL_Event event;

		// Handle events
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
			case SDL_EVENT_TERMINATING:
				{
					FCloseWindowEvent newEvent = {};
					engine->PushEvent(newEvent);
					break;
				}
			case SDL_EVENT_WINDOW_RESIZED:
				{
					FResizeWindowEvent newEvent = {};
					newEvent.mNewWindowSize = GetFrameBufferSize();
					engine->PushEvent(newEvent);
					break;
				}
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
				{
					OnSDLKeyboardEvent.ExecuteIfBound(engine, event.key);
					break;
				}
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					OnSDLMouseButtonEvent.ExecuteIfBound(engine, event.button);
					break;
				}
			case SDL_EVENT_MOUSE_MOTION:
				{
					OnSDLMouseMotionEvent.ExecuteIfBound(engine, event.motion);
					break;
				}
			case SDL_EVENT_MOUSE_WHEEL:
				{
					OnSDLMouseWheelEvent.ExecuteIfBound(engine, event.wheel);
					break;
				}
			default:
				break;
			}

			OnSDLEvent.Broadcast(this, &event);
		}
	}

	void Window::ShowWindow(bool bVisible)
	{
		TURBO_LOG(LogWindow, Info, "Setting window visibility to {}", bVisible);

		if (bVisible)
		{
			SDL_ShowWindow(mSDLWindow);
		}
		else
		{
			SDL_HideWindow(mSDLWindow);
		}
	}

	void Window::ShowCursor(bool bVisible)
	{
		TURBO_LOG(LogWindow, Display, "Setting cursor visibility to {}", bVisible);

		bool bResult = true;

		if (bVisible)
		{
			bResult &= SDL_ShowCursor();
		}
		else
		{
			bResult &= SDL_HideCursor();
		}

		bResult &= SDL_SetWindowRelativeMouseMode(mSDLWindow, !bVisible);

		if (bResult == false)
		{
			LogError();
		}
	}

	glm::uint2 Window::GetFrameBufferSize() const
	{
		glm::ivec2 Result;
		if (!SDL_GetWindowSizeInPixels(mSDLWindow, &Result.x, &Result.y))
		{
			LogError();
			Result = glm::ivec2(WindowDefaultValues::kSizeX, WindowDefaultValues::kSizeY);
		}

		return glm::ivec2(Result);
	}

	fp32 Window::GetDisplayScale() const
	{
		const fp32 displayScale = SDL_GetWindowDisplayScale(mSDLWindow);
		return displayScale > TURBO_SMALL_NUMBER ? displayScale : 1.f;
	}

	bool Window::IsFullscreenEnabled() const
	{
		return mbFullscreenEnabled;
	}

	void Window::SetFullscreen(bool bFullscreen)
	{
		if (!SDL_SetWindowFullscreen(mSDLWindow, bFullscreen))
		{
			LogError();
			return;
		}

		 mbFullscreenEnabled = bFullscreen;
	}

	void Window::SetWindowIcon(std::string_view path)
	{
		TURBO_LOG(LogWindow, Info, "Setting window icon {}", path);

		if (mWindowIconSurface)
		{
			TURBO_LOG(LogWindow, Info, "Destroying old window icon.", path);
			SDL_DestroySurface(mWindowIconSurface);
			mWindowIconSurface = nullptr;
		}

		mWindowIconSurface = LoadSurface(path.data());

		SDL_SetWindowIcon(mSDLWindow, mWindowIconSurface);
	}

	std::vector<const char*> Window::GetVulkanRequiredExtensions()
	{
		std::vector<const char*> Result;

		u32 ExtensionsCount;
		char const* const* ExtensionNames = SDL_Vulkan_GetInstanceExtensions(&ExtensionsCount);
		if (ExtensionNames == nullptr)
		{
			LogError();
			return Result;
		}

		for (int ExtensionId = 0; ExtensionId < ExtensionsCount; ++ExtensionId)
		{
			Result.push_back(ExtensionNames[ExtensionId]);
		}

		return Result;
	}

	bool Window::CreateVulkanSurface(VkInstance vulkanInstance)
	{
		if (mVulkanSurface)
		{
			return true;
		}

		if (!SDL_Vulkan_CreateSurface(mSDLWindow, vulkanInstance, nullptr, &mVulkanSurface))
		{
			TURBO_LOG(LogWindow, Error, "Window Vulkan surface creation error. Check bellow logs:");
			LogError();

			return false;
		}

		return true;
	}

	VkSurfaceKHR Window::GetVulkanSurface()
	{
		TURBO_CHECK(mVulkanSurface);

		return mVulkanSurface;
	}

	bool Window::DestroyVulkanSurface(VkInstance vulkanInstance)
	{
		if (vulkanInstance)
		{
			SDL_Vulkan_DestroySurface(vulkanInstance, mVulkanSurface, nullptr);
			mVulkanSurface = nullptr;

			return true;
		}

		return false;
	}
} // Turbo
