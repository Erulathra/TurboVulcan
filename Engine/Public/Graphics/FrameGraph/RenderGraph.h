#pragma once

#include "CommonMacros.h"
#include "CommonTypeDefs.h"
#include "Core/DataStructures/Handle.h"
#include "Core/Delegate.h"
#include "Core/Allocators/ArenaAllocator.h"
#include "Core/Memory.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/FrameGraph/RenderGraphHelpers.h"
#include "Graphics/Resources.h"

DECLARE_LOG_CATEGORY(LogRenderGraph, Info, Display)

namespace Turbo
{
	struct FRenderResources;
	class FCommandBuffer;
	class GPUDevice;
	struct FRGPassInfo;
	struct RenderGraph;
	struct FTexture;

	DECLARE_DELEGATE(FRGExecutePassDelegate, GPUDevice* /*gpu*/, FCommandBuffer& /*cmd*/, FRenderResources& /*resources*/);

	struct FRGPassInfo
	{
		FRGResourceHandle ReadTexture(FRGResourceHandle texture);
		FRGResourceHandle WriteTexture(FRGResourceHandle texture);

		FRGResourceHandle ReadBuffer(FRGResourceHandle buffer);
		FRGResourceHandle WriteBuffer(FRGResourceHandle buffer);

		void AddAttachment(FRGResourceHandle attachment, u32 attachmentIndex);
		void AddAttachment(FRGAttachment attachment, u32 attachmentIndex);
		void SetDepthStencilAttachment(FRGResourceHandle attachment);
		void SetDepthStencilAttachment(FRGAttachment attachment);

	public:
		std::vector<FRGResourceHandle> mTextureReads;
		std::vector<FRGResourceHandle> mTextureWrites;

		std::vector<FRGResourceHandle> mBufferReads;
		std::vector<FRGResourceHandle> mBufferWrites;

		std::array<FRGAttachment, kMaxColorAttachments> mColorAttachments;
		FRGAttachment mDepthStencilAttachment = {};

		EPassType mPassType = EPassType::Undefined;

		FRGExecutePassDelegate mExecutePass;

		RenderGraph* mGraphBuilder = nullptr;
		FRGPassHandle mHandle = {};
		FName mName = {};
	};

	struct FRGPassInitializer final
	{
		DELETE_COPY(FRGPassInitializer)

	public:
		explicit FRGPassInitializer(RenderGraph& graphBuilder, FRGPassInfo& passInfo);
		~FRGPassInitializer();

		[[nodiscard]] FRGPassInfo& Get() const;
		FRGPassInfo* operator->();
		const FRGPassInfo* operator->() const;

	private:
		RenderGraph* mOwner = nullptr;
		FRGPassHandle mHandle = {};

	public:
		friend struct RenderGraph;
	};

	struct FRenderResources
	{
      THandle<FDescriptorSet> mDescriptorSet = {};
      std::vector<THandle<FTexture>> mTextures;
      std::vector<THandle<FBuffer>> mBuffers;

      THandle<FTexture> GetTexture(FRGResourceHandle handle)
      {
         TURBO_CHECK(handle.IsValid() && handle.GetType() == ERGResourceType::Texture && handle.GetIndex() < mTextures.size())
         return mTextures[handle.GetIndex()];
      }

      THandle<FBuffer> GetBuffer(FRGResourceHandle handle)
      {
         TURBO_CHECK(handle.IsValid() && handle.GetType() == ERGResourceType::Buffer && handle.GetIndex() < mBuffers.size())
         return mBuffers[handle.GetIndex()];
      }
	};

	struct RenderGraph
	{
		static constexpr u32 kPerFrameStackSize = 64 * Memory::kMebi;
		static constexpr u32 kBufferAddressTableSize = 1024;
		static constexpr u32 kTextureBindingTableSize = 1024;

		/* Properties */
		GPUDevice* mGPU = nullptr;

		/* Render passes */
		std::vector<FRGPassInfo> mRenderPasses;

		/* Textures */
		std::vector<FRGTextureInfo> mTextures;
		using FRGPassTextureBarriers = std::vector<FRGTextureMemoryBarrier>;
		std::vector<FRGPassTextureBarriers> mPerPassTextureBarriers;
		FRGPassTextureBarriers mExternalTexturesBarriers;

		/* Buffer */
		std::vector<FRGBufferInfo> mBuffers;
		std::vector<FRGBufferUpload> mQueuedBufferUploads;

		using FRGPassBufferBarriers = std::vector<FRGBufferMemoryBarrier>;
		std::vector<FRGPassBufferBarriers> mPerPassBufferBarriers;

		/* Render graph Resources */
		THandle<FDescriptorPool> mDescriptorPool;
		THandle<FDescriptorSetLayout> mDescriptorSetLayout;
		std::array<THandle<FDescriptorSet>, kMaxFramesInFlight> mDescriptorSets;

		/* Allocator */
		FArenaAllocator mAllocator = FArenaAllocator(kPerFrameStackSize);

		/* API */

		DELETE_COPY(RenderGraph)
		RenderGraph() = default;

		void Init(GPUDevice* gpu);
		void Shutdown(GPUDevice* gpu);

		/* Texture related methods */
		[[nodiscard]] FRGResourceHandle CreateTexture(const FRGTextureInfo& textureInfo);
		FRGResourceHandle RegisterExternalTexture(THandle<FTexture> texture, ETextureLayout initLayout);
		FRGResourceHandle RegisterExternalTexture(THandle<FTexture> textureHandle, ETextureLayout initLayout, ETextureLayout finalLayout);
		[[nodiscard]] FRGTextureInfo GetTextureInfo(FRGResourceHandle resourceHandle) const;

		/* Buffer related methods */
		[[nodiscard]] FRGResourceHandle CreateBuffer(const FRGBufferInfo& bufferInfo);
		FRGResourceHandle RegisterExternalBuffer(THandle<FBuffer> buffer);
		void QueueBufferUpload(const FRGBufferUpload& bufferUpload);
		[[nodiscard]] std::tuple<FRGResourceHandle, void* /*intermediatePtr */> CreateAndQueueBufferUpload(const FCreateAndUploadBuffer& createAndUploadBuffer);
		[[nodiscard]] FRGBufferInfo GetBufferInfo(FRGResourceHandle resourceHandle) const;

		template<typename T>
		[[nodiscard]] std::tuple<FRGResourceHandle, T* /*intermediatePtr */> CreateAndQueueBufferUpload(const FCreateAndUploadBuffer& createAndUploadBuffer)
		{
			std::tuple<FRGResourceHandle, void*> result = CreateAndQueueBufferUpload(createAndUploadBuffer);
			return std::make_tuple(std::get<0>(result), static_cast<T*>(std::get<1>(result)));
		}

		/* Pass related methods */
		[[nodiscard]] FRGPassInitializer AddPass(FName passName, EPassType passType = EPassType::Undefined);

		/* Compilation */
		void Compile();
		void CompileTextureSynchronization();
		void CompileBufferSynchronization();

		/* Execution */
		void Execute(GPUDevice* gpu, FCommandBuffer& cmd);
		void Reset();

		/* Stack allocation Interface */
		[[nodiscard]] void* Allocate(SizeType numBytes) { return Memory::Allocate(&mAllocator, numBytes); }

		template <typename PODType>
		[[nodiscard]] PODType* AllocatePOD() { return Memory::Allocate<PODType>(&mAllocator); }

		template <typename PODType>
		[[nodiscard]] PODType* AllocatePOD(SizeType num) { return Memory::Allocate<PODType>(&mAllocator, num); }

		/* Other */
		[[nodiscard]] vk::Format GetTextureFormat(FRGResourceHandle resourceHandle) const;
	};
} // Turbo
