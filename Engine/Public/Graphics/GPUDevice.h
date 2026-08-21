#pragma once

#include "CommandBuffer.h"
#include "Core/Allocators/ArenaAllocator.h"
#include "Core/DataStructures/Handle.h"
#include "Graphics/GraphicsCore.h"
#include "Resources.h"
#include "VkBootstrap.h"

#include "DestoryQueue.h"
#include "ResourceBuilders.h"
#include "VulkanHelpers.h"
#include "Core/DataStructures/GenPool.h"
#include "Graphics/Resources.h"
#include <vector>

DECLARE_LOG_CATEGORY(LogGPUDevice, Info, Display)

namespace Turbo
{
	class FWindow;

	DECLARE_DELEGATE(FOnImmediateSubmit, FCommandBuffer&);

	constexpr size_t kBufferPoolSize = 16384;
	constexpr size_t kTexturePoolSize = 1024;
	constexpr size_t kSamplerPoolSize = 128;
	constexpr size_t kPipelinePoolSize = 256;
	constexpr size_t kBLASPoolSize = 1024;
	constexpr size_t kTLASPoolSize = 16;

	constexpr u32 kInvalidBinding = std::numeric_limits<u32>::max();

	struct FBufferedFrameData final
	{
		vk::Fence mCommandBufferExecutedFence = nullptr;
		vk::Semaphore mImageAcquiredSemaphore = nullptr;

		std::array<vk::CommandPool, kMaxRenderingThreads> mVkCommandPools;

		TUniquePtr<FCommandBuffer> mMainCommandBuffer;

		FDestroyQueue mDestroyQueue;
	};

	class FGPUDevice final
	{
		/** Initialization interface */
	public:
		void Init(const FGPUDeviceBuilder& gpuDeviceBuilder);
		void Shutdown();
		/** Initialization interface end */

		/** Rendering interface */
	public:
		bool BeginFrame();
		bool PresentFrame();

		[[nodiscard]] const FBufferedFrameData& GetFrameData(u32 index) const { return mFrameDatas[index]; }
		[[nodiscard]] vk::CommandPool GetCommandPool() const;
		[[nodiscard]] FCommandBuffer& GetMainCommandBuffer() const;

		[[nodiscard]] THandle<FDescriptorSet> GetBindlessResourcesSet() const { return mBindlessResourcesSet; }
		[[nodiscard]] THandle<FTexture> GetPresentImage() const { return mSwapChainTextures[mCurrentSwapchainImageIndex]; }

		void WaitIdle() const;

		void ImmediateSubmit(const FOnImmediateSubmit& immediateSubmitDelegate);
		void SubmitMainCommandBufferAndWaitIdle();

		[[nodiscard]] glm::uint2 GetMainViewportSize() const { return mViewportSize; }
		void SetMainViewportSize(glm::uint2 newSize)
		{
			TURBO_CHECK(newSize.x > 0 && newSize.y > 0)
			mViewportSize = newSize;
		}

		[[nodiscard]] glm::uint2 GetFrameBufferSize() const { return mFramebufferSize; }
		[[nodiscard]] u32 GetFrameInFlightId() const { return mBufferedFrameId; }
		[[nodiscard]] u32 GetNumRenderedFrames() const { return mRenderedFrames; }
		[[nodiscard]] u32 GetNumBufferedFrames() const { return kMaxFramesInFlight; }
		[[nodiscard]] u32 GetNumRenderingThreads() const { return mNumRenderingThreads; }

		void RequestSwapChainResize() { mbRequestedSwapchainResize = true; }

		void RecreatePipelines();
		void RecompileShaders();

		/** Rendering interface end */

		/** Resource accessors */
	public:
		[[nodiscard]] FBuffer* AccessBuffer(THandle<FBuffer> handle) { return mBufferPool.Get(handle); }
		[[nodiscard]] FTexture* AccessTexture(THandle<FTexture> handle) { return mTexturePool.Get(handle); }
		[[nodiscard]] FSampler* AccessSampler(THandle<FSampler> handle) { return mSamplerPool.Get(handle); }
		[[nodiscard]] FPipeline* AccessPipeline(THandle<FPipeline> handle) { return mPipelinePool.Get(handle); }
		[[nodiscard]] FDescriptorPool* AccessDescriptorPool(THandle<FDescriptorPool> handle) { return mDescriptorPoolPool.Get(handle); }
		[[nodiscard]] FDescriptorSetLayout* AccessDescriptorSetLayout(THandle<FDescriptorSetLayout> handle) { return mDescriptorSetLayoutPool.Get(handle); }
		[[nodiscard]] FDescriptorSet* AccessDescriptorSet(THandle<FDescriptorSet> handle) { return mDescriptorSetPool.Get(handle); }
		[[nodiscard]] FShaderState* AccessShaderState(THandle<FShaderState> handle) { return mShaderStatePool.Get(handle); }
		[[nodiscard]] FBLAS* AccessBLAS(THandle<FBLAS> handle) { return mBLASPool.Get(handle); }
		[[nodiscard]] FTLAS* AccessTLAS(THandle<FTLAS> handle) { return mTLASPool.Get(handle); }

		[[nodiscard]] const FBuffer* AccessBuffer(THandle<FBuffer> handle) const { return mBufferPool.Get(handle); }
		[[nodiscard]] const FTexture* AccessTexture(THandle<FTexture> handle) const { return mTexturePool.Get(handle); }
		[[nodiscard]] const FSampler* AccessSampler(THandle<FSampler> handle) const { return mSamplerPool.Get(handle); }
		[[nodiscard]] const FPipeline* AccessPipeline(THandle<FPipeline> handle) const { return mPipelinePool.Get(handle); }
		[[nodiscard]] const FDescriptorPool* AccessDescriptorPool(THandle<FDescriptorPool> handle) const { return mDescriptorPoolPool.Get(handle); }
		[[nodiscard]] const FDescriptorSetLayout* AccessDescriptorSetLayout(THandle<FDescriptorSetLayout> handle) const { return mDescriptorSetLayoutPool.Get(handle); }
		[[nodiscard]] const FDescriptorSet* AccessDescriptorSet(THandle<FDescriptorSet> handle) const { return mDescriptorSetPool.Get(handle); }
		[[nodiscard]] const FShaderState* AccessShaderState(THandle<FShaderState> handle) const { return mShaderStatePool.Get(handle); }
		[[nodiscard]] const FBLAS* AccessBLAS(THandle<FBLAS> handle) const { return mBLASPool.Get(handle); }
		[[nodiscard]] const FTLAS* AccessTLAS(THandle<FTLAS> handle) const { return mTLASPool.Get(handle); }

		/** Resource accessors end */

		/** Resource creation */
	public:
		THandle<FBuffer> CreateBuffer(const FBufferBuilder& builder);
		THandle<FTexture> CreateTexture(const FTextureBuilder& builder);
		THandle<FSampler> CreateSampler(const FSamplerBuilder& builder);
		THandle<FPipeline> CreatePipeline(const FPipelineBuilder& builder);
		THandle<FDescriptorPool> CreateDescriptorPool(const FDescriptorPoolBuilder& builder);
		THandle<FDescriptorSetLayout> CreateDescriptorSetLayout(const FDescriptorSetLayoutBuilder& builder);
		THandle<FDescriptorSet> CreateDescriptorSet(const FDescriptorSetBuilder& builder);
		THandle<FShaderState> CreateShaderState(const FShaderStateBuilder& builder);
		THandle<FBLAS> CreateBLAS(const FBLASBuilder& builder);
		THandle<FTLAS> CreateTLAS(const FTLASBuilder& builder);

		vk::CommandPool CreateCommandPool(u32 queueFamilyIndex, vk::CommandPoolCreateFlags createFlags = {});
		TUniquePtr<FCommandBuffer> CreateCommandBuffer(const FCommandBufferBuilder& builder);

		/** Resource creation end */

		/** Other resource related methods */
	public:
   	[[nodiscard]] FAccelerationStructureSizeInfo CalculateTLASSize(const FTLASBuilder& builder) const;
		void ResetDescriptorPool(THandle<FDescriptorPool> descriptorPoolHandle);
		/** Other resource related methods end */

		/** Resource destroy */
	public:
		void DestroyBuffer(THandle<FBuffer> handle);
		void DestroyTexture(THandle<FTexture> handle);
		void DestroySampler(THandle<FSampler> handle);
		void DestroyPipeline(THandle<FPipeline> handle);
		void DestroyDescriptorPool(THandle<FDescriptorPool> handle);
		void DestroyDescriptorSetLayout(THandle<FDescriptorSetLayout> handle);
		void DestroyShaderState(THandle<FShaderState> handle);

		void DestroyBLAS(THandle<FBLAS> handle);
		void DestroyTLAS(THandle<FTLAS> handle);
		void DestroyAccelerationStructure(FHandle handle, FAccelerationStructure* accelerationStructure);

		void AddOnDestroyCallback(FOnDestroy::Delegate&& delegate);
		/** Resource destroy end */

		/** Destroy immediate */
	public:
		void DestroyBufferImmediate(const FBufferDestroyer& destroyer);
		void DestroyTextureImmediate(const FTextureDestroyer& destroyer);
		void DestroySamplerImmediate(const FSamplerDestroyer& destroyer);
		void DestroyPipelineImmediate(const FPipelineDestroyer& destroyer);
		void DestroyDescriptorPoolImmediate(const FDescriptorPoolDestroyer& destroyer);
		void DestroyDescriptorSetLayoutImmediate(const FDescriptorSetLayoutDestroyer& destroyer);
		void DestroyShaderStateImmediate(const FShaderStateDestroyer& destroyer);
		void DestroyAccelerationStructureImmediate(const FAccelerationStructureDestroyer& destroyer);

		/** Destroy immediate end */

		/** Resource helpers */
	public:
		void UploadTextureUsingStagingBuffer(THandle<FTexture> handle, std::span<const u8> data);

		/** Resource helpers end */

		/** Vulkan Getters */
	public:
		[[nodiscard]] vk::Instance GetVkInstance() const { return mVkInstance; }
		[[nodiscard]] vk::PhysicalDevice GetVkPhysicalDevice() const { return mVkPhysicalDevice; }
		[[nodiscard]] vk::Device GetVkDevice() const { return mVkDevice; }
		[[nodiscard]] vk::Queue GetVkQueue() const { return mVkGraphicsQueue; }

		[[nodiscard]] u32 GetGraphicsQueueFamily() const { return mVkGraphicsQueueFamilyIndex; }
		[[nodiscard]] u32 GetComputeQueueFamily() const { return mVkComputeQueueFamilyIndex; }
		[[nodiscard]] u32 GetTransferQueueFamily() const { return mVkTransferQueueFamilyIndex; }

		/** Vulkan Getters end */

#if WITH_PROFILER
		/** Profiling */
		[[nodiscard]] FTraceGPUCtx GetTraceGpuCtx() const { return mTraceGpuCtx; }
#endif

		/** Initialization methods */
	private:
		vkb::Instance CreateVkInstance(const std::vector<ConstString>& requiredExtensions);
		vkb::PhysicalDevice SelectPhysicalDevice(const vkb::Instance& builtInstance);
		vkb::Device CreateDevice(const vkb::PhysicalDevice& physicalDevice);
		vkb::Swapchain CreateSwapchain();
		void CreateVulkanMemoryAllocator();
		void CreateFrameDatas();

		void InitializeImmediateCommands();

		void InitializeBindlessResources();

		vk::PresentModeKHR GetBestPresentMode();
		/** Initialization methods end */

	private:
		void ResizeSwapChain();

		/** Destroy methods */
	private:
		void DestroySwapChain();
		void DestroyFrameDatas();
		void DestroyImmediateCommands();
		void DestroyBindlessResources();
		void FlushDestroyQueues();

		/** Destroy methods end */

		/** Utils */
	private:
		/** Utils end */

		/** Rendering interface */
	private:
		void UpdateBindlessResources();

		/** Rendering interface end */

		/** Creation helpers */
	private:
		void InitVulkanTexture(const FTextureBuilder& builder, THandle<FTexture> handle);
		void InitPipeline(const FPipelineBuilder& builder, THandle<FPipeline> handle);

		/** Creation helpers end */

		/** Debug */
	private:
		static VkBool32 ValidationLayerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData
		);

		template<typename HandleType>
		void SetResourceName(HandleType vkHandle, FName name) const;

		template<typename HandleType>
		void SetResourceName(HandleType vkHandle, std::string_view name) const;
		/* Debug end */

		/* Resource pools */
	private:
		TGenPool<FBuffer, kBufferPoolSize> mBufferPool;
		TGenPool<FTexture, kTexturePoolSize> mTexturePool;
		TGenPool<FSampler, kSamplerPoolSize> mSamplerPool;
		TGenPool<FPipeline, kPipelinePoolSize> mPipelinePool;
		TGenPool<FDescriptorSetLayout, 128> mDescriptorSetLayoutPool;
		TGenPool<FDescriptorPool, 16> mDescriptorPoolPool;
		TGenPool<FDescriptorSet, 256> mDescriptorSetPool;
		TGenPool<FShaderState, 256> mShaderStatePool;
		TGenPool<FBLAS, kBLASPoolSize> mBLASPool;
		TGenPool<FTLAS, kTLASPoolSize> mTLASPool;

		/* Resource pools end */

		/* Bindless resources */
	private:
		THandle<FDescriptorPool> mBindlessResourcesPool;
		THandle<FDescriptorSetLayout> mBindlessResourcesLayout;
		THandle<FDescriptorSet> mBindlessResourcesSet;

		std::vector<FBindlessResourceUpdateRequest> mBindlessResourcesToUpdate;

		/* Bindless resources end */

		/* Vulkan handles */
	private:
		vk::Instance mVkInstance = nullptr;

		vk::PhysicalDevice mVkPhysicalDevice = nullptr;
		vk::PhysicalDeviceProperties mVkPhysicalDeviceProperties = {};
		vk::PhysicalDeviceAccelerationStructurePropertiesKHR mVkAccelerationStructureProperties = {};
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR mVkRayTracingPipelineProperties = {};

		vk::Device mVkDevice = nullptr;

		vk::Queue mVkGraphicsQueue = nullptr;
		u32 mVkGraphicsQueueFamilyIndex = std::numeric_limits<u32>::max();

		vk::Queue mVkTransferQueue = nullptr;
		u32 mVkTransferQueueFamilyIndex = std::numeric_limits<u32>::max();

		vk::Queue mVkComputeQueue = nullptr;
		u32 mVkComputeQueueFamilyIndex = std::numeric_limits<u32>::max();

		vk::DescriptorPool mVkDescriptorPool = nullptr;

		vma::Allocator mVmaAllocator = nullptr;

		/** Vulkan Handles end */

		/** Swapchain */
	private:
		vk::SwapchainKHR mVkSwapchain = nullptr;
		vk::SurfaceKHR mVkWindowSurface = nullptr;
		vk::SurfaceFormatKHR mVkSurfaceFormat = {};
		vk::PresentModeKHR mPresentMode = {};

		std::array<THandle<FTexture>, kMaxSwapChainImages> mSwapChainTextures;
		std::array<vk::Semaphore, kMaxSwapChainImages> mSubmitSemaphores;
		glm::uint2 mFramebufferSize = glm::uint2(0);

		u32 mNumSwapChainImages = 0;
		/** Note that this is an index of swap chain image */
		u32 mCurrentSwapchainImageIndex = 0;

		bool mbRequestedSwapchainResize = false;

		/** Swapchain end */

		/** Frame handing */
		u32 mNumRenderingThreads = 1;
		std::array<FBufferedFrameData, kMaxFramesInFlight> mFrameDatas;

		/** Note that this is an index of buffered frame */
		u32 mBufferedFrameId = 0;
		/** Note that this is an index of rendered frame (from Init) */
		u32 mRenderedFrames = 0;

		/** TODO: move me to better category */
		bool mbVSync = false;

		/** Frame handing */

		/** Immediate commands */
	private:
		vk::Fence mImmediateCommandsFence;
		vk::CommandPool mImmediateCommandsPool;
		TUniquePtr<FCommandBuffer> mImmediateCommandsBuffer;

		/** Immediate commands end */

		/** Profiling */
	private:
		FTraceGPUCtx mTraceGpuCtx = {};
		/** Profiling end */

		/** Other */
	private:
      FArenaAllocator mPerFrameArena{16 * Memory::kKibi};

		FDestroyQueue mDestroyQueue;
		vk::DebugUtilsMessengerEXT mVkDebugUtilsMessenger;

		glm::uint2 mViewportSize = glm::uint2(0);
		EMSAASamples mMaxSupportedSampleCount = EMSAASamples::One;

		/** Other end */

	private:
		FGPUDevice() = default;

	public:
		DELETE_COPY(FGPUDevice);

	public:
		friend class Engine;
	};


	template <typename HandleType>
	void FGPUDevice::SetResourceName(HandleType vkHandle, FName name) const
	{
		SetResourceName(vkHandle, name.ToString());
	}

	template <typename HandleType>
	void FGPUDevice::SetResourceName(HandleType vkHandle, const std::string_view name) const
	{
#if WITH_DEBUG_RENDERING_FEATURES
		vk::DebugUtilsObjectNameInfoEXT nameInfo = {};
		nameInfo.objectType = vkHandle.objectType;
		nameInfo.objectHandle = HandleTraits<HandleType>::CastToU64Handle(vkHandle);
		const std::string objectName = std::string(name) + std::string(HandleTraits<HandleType>::GetTypePostFix());
		nameInfo.pObjectName = objectName.c_str();
		CHECK_VULKAN_HPP(mVkDevice.setDebugUtilsObjectNameEXT(nameInfo));
#endif // WITH_DEBUG_RENDERING_FEATURES
	}
} // Turbo
