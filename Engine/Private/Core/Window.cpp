#include "Core/Window.h"

#include "Core/Engine.h"
#include "SDL3/SDL_vulkan.h"

namespace Turbo
{
	std::unique_ptr<Window> Window::MainWindow = nullptr;

	Window::Window()
	{
	}

	Window::~Window()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Destroying window.");

		SDL_DestroyWindow(SDLContext.Window);
	}

	void Window::InitBackend()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Initializing SDL.");
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			LogError();
		}
	}

	bool Window::CreateMainWindow()
	{
		MainWindow = std::unique_ptr<Window>(new Window());
		return MainWindow->InitWindow();
	}

	void Window::DestroyMainWindow()
	{
		MainWindow.reset();

		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Stopping SDL.");
		SDL_Quit();
	}

	bool Window::InitWindow()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Initializing Window.");
		SDLContext.Window = SDL_CreateWindow(WindowDefaultValues::Name.c_str(), WindowDefaultValues::SizeX, WindowDefaultValues::SizeY,
		                                     SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
		if (!SDLContext.Window)
		{
			TURBO_LOG(LOG_WINDOW, LOG_ERROR, "SDL window creation error. See bellow logs for details");
			LogError();

			return false;
		}

		return true;
	}

	void Window::LogError()
	{
		TURBO_LOG(LOG_WINDOW, LOG_ERROR, "SDL_ERROR: {}", SDL_GetError());
	}

	void Window::PollWindowEventsAndErrors()
	{
		SDL_Event WindowEvent;
		while (SDL_PollEvent(&WindowEvent))
		{
			if (WindowEvent.type == SDL_EVENT_QUIT || WindowEvent.type == SDL_EVENT_TERMINATING)
				OnWindowEvent(EWindowEvent::WindowCloseRequest);
			else if (WindowEvent.type == SDL_EVENT_WINDOW_FOCUS_LOST)
				OnWindowEvent(EWindowEvent::FocusLost);
			else if (WindowEvent.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
				OnWindowEvent(EWindowEvent::FocusGained);
		}
	}

	void Window::ShowWindow(bool bVisible)
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Setting window visibility to {}", bVisible);

		if (bVisible)
		{
			SDL_ShowWindow(SDLContext.Window);
		}
		else
		{
			SDL_HideWindow(SDLContext.Window);
		}
	}

	void Window::InitForVulkan()
	{
		if (!SDL_Vulkan_LoadLibrary(nullptr))
		{
			LogError();
		}
	}

	void Window::DeInitForVulkan()
	{
		SDL_Vulkan_UnloadLibrary();
	}

	std::vector<const char*> Window::GetVulkanExtensions()
	{
		std::vector<const char*> Result;

		uint32 ExtensionsCount;
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

	bool Window::CreateVulkanSurface(VkInstance VulkanInstance, VkSurfaceKHR& OutVulkanSurface) const
	{
		if (!SDL_Vulkan_CreateSurface(SDLContext.Window, VulkanInstance, nullptr, &OutVulkanSurface))
		{
			TURBO_LOG(LOG_WINDOW, LOG_ERROR, "Window Vulkan surface creation error. Check bellow logs:");
			LogError();

			return false;
		}

		return true;
	}

	bool Window::DestroyVulkanSurface(VkInstance VulkanInstance, VkSurfaceKHR Surface)
	{
		if (VulkanInstance)
		{
			SDL_Vulkan_DestroySurface(VulkanInstance, Surface, nullptr);
			return true;
		}

		return false;
	}
} // Turbo
