#include "Core/RHI/VulkanRHI.h"

#include <fmt/format.h>

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/RHI/VulkanHardwareDevice.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/SwapChain.h"

auto fmt::formatter<VkResult, char, void>::format(VkResult Result, format_context& CTX) const
    -> format_context::iterator
{
    return formatter<int32>::format(Result, CTX);
}

namespace Turbo
{
    FVulkanRHI::FVulkanRHI() = default;
    FVulkanRHI::~FVulkanRHI() = default;

    void FVulkanRHI::Init()
    {
        CreateVulkanInstance();
        gEngine->GetWindow()->CreateVulkanSurface(mVulkanInstance);
        AcquirePhysicalDevice();

        if (IsValid(mHardwareDevice))
        {
            mDevice = std::make_unique<FVulkanDevice>();
            mDevice->Init(mHardwareDevice.get());
        }

        if (IsValid(mDevice))
        {
            mSwapChain = std::make_unique<FSwapChain>();
            mSwapChain->Init(mDevice.get());
        }
    }

    void FVulkanRHI::InitWindow(FSDLWindow* Window)
    {
        Window->InitForVulkan();
    }

    void FVulkanRHI::Destroy()
    {
        if (mSwapChain)
        {
            mSwapChain->Destroy();
            mSwapChain.reset();
        }

        if (mDevice)
        {
            mDevice->Destroy();
            mDevice.reset();
        }

        mHardwareDevice.reset();

        gEngine->GetWindow()->DestroyVulkanSurface(mVulkanInstance);
        DestroyVulkanInstance();
    }

    void FVulkanRHI::CreateVulkanInstance()
    {
        TURBO_LOG(LOG_RHI, LOG_INFO, "Initialize VOLK");
        if (VkResult VolkInitializeResult = volkInitialize(); VolkInitializeResult != VK_SUCCESS)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "VOLK initialization error. (Error: {})", VolkInitializeResult);
            gEngine->RequestExit(EExitCode::RHICriticalError);
            return;
        }

        VkApplicationInfo AppInfo{};
        AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        AppInfo.pApplicationName = "Turbo Vulkan";
        AppInfo.applicationVersion = TURBO_VERSION();
        AppInfo.pEngineName = "Turbo Vulkan";
        AppInfo.engineVersion = TURBO_VERSION();
        AppInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo CreateInfo{};
        CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        CreateInfo.pApplicationInfo = &AppInfo;
        CreateInfo.pNext = nullptr;

        std::vector<const char*> ExtensionNames = gEngine->GetWindow()->GetVulkanRequiredExtensions();

#if WITH_VALIDATION_LAYERS
        if (CheckValidationLayersSupport())
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Validation layers supported.")

            CreateInfo.enabledLayerCount = VulkanValidationLayers.size();
            CreateInfo.ppEnabledLayerNames = VulkanValidationLayers.data();

            ExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
#endif // else !WITH_VALIDATION_LAYERS
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Validation layers are unsupported or disabled.")

            CreateInfo.enabledLayerCount = 0;
            CreateInfo.ppEnabledLayerNames = nullptr;
        }

        CreateInfo.enabledExtensionCount = ExtensionNames.size();
        CreateInfo.ppEnabledExtensionNames = ExtensionNames.data();

        TURBO_LOG(LOG_RHI, LOG_INFO, "Creating VKInstance.")
        const VkResult CreationResult = vkCreateInstance(&CreateInfo, nullptr, &mVulkanInstance);
        if (CreationResult != VK_SUCCESS)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "VKInstance creation failed. (Error: {})", CreationResult);
            gEngine->RequestExit(EExitCode::RHICriticalError);
            return;
        }

        volkLoadInstanceOnly(mVulkanInstance);

#if WITH_VALIDATION_LAYERS
        SetupValidationLayersCallbacks();
#endif // WITH_VALIDATION_LAYERS

        EnumerateVulkanExtensions();

        std::stringstream ExtensionsStream;
        for (const VkExtensionProperties& Extension : mExtensionProperties)
        {
            ExtensionsStream << "\t" << Extension.extensionName << "\n";
        }
        TURBO_LOG(LOG_RHI, LOG_DISPLAY, "Supported Extensions: \n {}", ExtensionsStream.str());
    }

    void FVulkanRHI::DestroyVulkanInstance()
    {
        if (mVulkanInstance)
        {
#if WITH_VALIDATION_LAYERS
            DestroyValidationLayersCallbacks();
#endif // WITH_VALIDATION_LAYERS

            TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying VKInstance.")
            vkDestroyInstance(mVulkanInstance, nullptr);
            mVulkanInstance = nullptr;
        }
    }

    void FVulkanRHI::EnumerateVulkanExtensions()
    {
        TURBO_CHECK(mVulkanInstance);

        uint32 ExtensionsNum;
        vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionsNum, nullptr);
        mExtensionProperties.resize(ExtensionsNum);
        uint32 Result = vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionsNum, mExtensionProperties.data());
        if (Result != VK_SUCCESS)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "Vulkan extensions enumeration error: {0:X}", Result);
        }
    }

#if WITH_VALIDATION_LAYERS

    bool FVulkanRHI::CheckValidationLayersSupport()
    {
        uint32 LayerPropertiesNum;
        vkEnumerateInstanceLayerProperties(&LayerPropertiesNum, nullptr);

        std::vector<VkLayerProperties> LayerProperties;
        LayerProperties.reserve(LayerPropertiesNum);
        vkEnumerateInstanceLayerProperties(&LayerPropertiesNum, LayerProperties.data());

        bool bSuccess = true;

        for (const char* RequestedValidationLayer : VulkanValidationLayers)
        {
            for (VkLayerProperties LayerProperty : LayerProperties)
            {
                if (std::strcmp(LayerProperty.layerName, RequestedValidationLayer) != 0)
                {
                    TURBO_LOG(LOG_RHI, LOG_CRITICAL, "Missing Validation Layer: {}", RequestedValidationLayer);
                    bSuccess = false;
                }
            }
        }

        if (bSuccess)
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "All requested validation layers are supported.");
        }

        return bSuccess;
    }

    void FVulkanRHI::SetupValidationLayersCallbacks()
    {
        TURBO_LOG(LOG_RHI, LOG_INFO, "Assigning validation layers callback.");

        VkDebugUtilsMessengerCreateInfoEXT CreateInfo{};
        CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        CreateInfo.messageSeverity =
              VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        CreateInfo.messageType =
              VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        CreateInfo.pfnUserCallback = HandleValidationLayerCallback;
        CreateInfo.pUserData = nullptr;

        vkCreateDebugUtilsMessengerEXT(mVulkanInstance, &CreateInfo, nullptr, &mDebugMessengerHandle);
    }

    void FVulkanRHI::DestroyValidationLayersCallbacks()
    {
        if (mDebugMessengerHandle)
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying validation layers callback.");
            vkDestroyDebugUtilsMessengerEXT(mVulkanInstance, mDebugMessengerHandle, nullptr);
        }
    }

    VkBool32 FVulkanRHI::HandleValidationLayerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT MessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
        void* UserData)
    {
        if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            TURBO_LOG(LOG_RHI, LOG_DISPLAY, "{}", CallbackData->pMessage)
        }
        else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            // Messages with info severity are very verbose, so I reduced its verbosity to display.
            TURBO_LOG(LOG_RHI, LOG_DISPLAY, "{}", CallbackData->pMessage)
        }
        else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            TURBO_LOG(LOG_RHI, LOG_WARN, "{}", CallbackData->pMessage)
        }
        else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "{}", CallbackData->pMessage)
        }

        return VK_FALSE;
    }

#endif // WITH_VALIDATION_LAYERS

    void FVulkanRHI::AcquirePhysicalDevice()
    {
        uint32 PhysicalDeviceNum;
        vkEnumeratePhysicalDevices(mVulkanInstance, &PhysicalDeviceNum, nullptr);

        if (PhysicalDeviceNum != 0)
        {
            std::vector<VkPhysicalDevice> FoundVulkanDevices(PhysicalDeviceNum);
            vkEnumeratePhysicalDevices(mVulkanInstance, &PhysicalDeviceNum, FoundVulkanDevices.data());

            std::vector<FVulkanHardwareDevice*> HardwareDevices;
            for (VkPhysicalDevice VulkanDevice : FoundVulkanDevices)
            {
                FVulkanHardwareDevice* HWDevice = new FVulkanHardwareDevice(VulkanDevice);
                if (HWDevice->IsValid())
                {
                    HardwareDevices.push_back(HWDevice);
                }
            }

            std::ranges::sort(
                HardwareDevices, std::ranges::greater{},
                [this](const FVulkanHardwareDevice* Device)
                {
                    return Device->CalculateDeviceScore();
                });

            mHardwareDevice = !HardwareDevices.empty() ? std::unique_ptr<FVulkanHardwareDevice>(HardwareDevices[0]) : nullptr;

            for (int DeviceId = 1; DeviceId < HardwareDevices.size(); ++DeviceId)
            {
                delete HardwareDevices[DeviceId];
                HardwareDevices[DeviceId] = nullptr;
            }
        }

        if (!IsValid(mHardwareDevice))
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "There is no suitable GPU device.");
            gEngine->RequestExit(EExitCode::DeviceNotSupported);
            return;
        }

        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(mHardwareDevice->GetVulkanPhysicalDevice(), &DeviceProperties);

        TURBO_LOG(LOG_RHI, LOG_INFO, "Using \"{}\" as primary physical device. (Score: {})", DeviceProperties.deviceName, mHardwareDevice->CalculateDeviceScore());
    }
} // Turbo
