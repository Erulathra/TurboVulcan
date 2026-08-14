#include "Graphics/FrameGraph/RenderGraph.h"

#include "CommonMacros.h"
#include "Core/DataStructures/Handle.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/Debug.h"
#include "Graphics/Enums.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/FrameGraph/RenderGraphHelpers.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/ResourceBuilders.h"
#include "Graphics/Resources.h"
#include "ProfilingMacros.h"
#include "TurboLog.h"
#include "entt/locator/locator.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_to_string.hpp"
#include <array>
#include <iterator>
#include <vector>

namespace Turbo
{
	FRGResourceHandle FRGPassInfo::ReadTexture(FRGResourceHandle texture)
	{
		TURBO_CHECK(texture.IsValid())
		TURBO_CHECK(texture.GetType() == ERGResourceType::Texture)
		TURBO_CHECK_SLOW(std::ranges::find(mTextureReads, texture) == mTextureReads.end())
		mTextureReads.emplace_back(texture);
		return texture;
	}

	FRGResourceHandle FRGPassInfo::WriteTexture(FRGResourceHandle texture)
	{
		TURBO_CHECK(texture.IsValid())
		TURBO_CHECK(texture.GetType() == ERGResourceType::Texture)
		TURBO_CHECK_SLOW(std::ranges::find(mTextureWrites, texture) == mTextureWrites.end())
		mTextureWrites.emplace_back(texture);
		return texture;
	}

	FRGResourceHandle FRGPassInfo::ReadBuffer(FRGResourceHandle buffer)
	{
		TURBO_CHECK(buffer.IsValid())
		TURBO_CHECK(buffer.GetType() == ERGResourceType::Buffer)
		TURBO_CHECK_SLOW(std::ranges::find(mBufferReads, buffer) == mBufferReads.end())
		mBufferReads.emplace_back(buffer);
		return buffer;
	}

	FRGResourceHandle FRGPassInfo::WriteBuffer(FRGResourceHandle buffer)
	{
		TURBO_CHECK(buffer.IsValid())
		TURBO_CHECK(buffer.GetType() == ERGResourceType::Buffer)
		TURBO_CHECK_SLOW(std::ranges::find(mBufferWrites, buffer) == mBufferWrites.end())
		mBufferWrites.emplace_back(buffer);
		return buffer;
	}

	void FRGPassInfo::AddAttachment(FRGResourceHandle attachment, uint32 attachmentIndex)
	{
		AddAttachment({.mTexture = attachment }, attachmentIndex);
	}

	void FRGPassInfo::AddAttachment(FRGAttachment attachment, uint32 attachmentIndex)
	{
		TURBO_CHECK(attachmentIndex < kMaxColorAttachments);
		TURBO_CHECK(mColorAttachments[attachmentIndex].IsValid() == false)

		WriteTexture(attachment.mTexture);
		if (attachment.mResolveTexture.IsValid())
		{
			WriteTexture(attachment.mResolveTexture);
		}

		mColorAttachments[attachmentIndex] = attachment;
	}

	void FRGPassInfo::SetDepthStencilAttachment(FRGResourceHandle attachment)
	{
		SetDepthStencilAttachment({
			.mTexture = attachment,
			.mClearColor = EClearColor::Zero
		});
	}

	void FRGPassInfo::SetDepthStencilAttachment(FRGAttachment attachment)
	{
		TURBO_CHECK(mDepthStencilAttachment.IsValid() == false)

		WriteTexture(attachment.mTexture);
		if (attachment.mResolveTexture.IsValid())
		{
			WriteTexture(attachment.mResolveTexture);
		}

		mDepthStencilAttachment = attachment;
	}

	FRGPassInitializer::FRGPassInitializer(FRenderGraphBuilder& graphBuilder, FRGPassInfo& passInfo)
		: mOwner(&graphBuilder)
		, mHandle(passInfo.mHandle)
	{
	}

	FRGPassInitializer::~FRGPassInitializer()
	{
		const FRGPassInfo& pass = Get();

		TURBO_CHECK(pass.mPassType != EPassType::Undefined)
		TURBO_CHECK_MSG(pass.mExecutePass.IsBound(), "{}: Execute delegate is not bound", pass.mName)
	}

	FRGPassInfo& FRGPassInitializer::Get() const
	{
		return mOwner->mRenderPasses[mHandle.mIndex];
	}

	FRGPassInfo* FRGPassInitializer::operator->()
	{
		return &mOwner->mRenderPasses[mHandle.mIndex];
	}

	const FRGPassInfo* FRGPassInitializer::operator->() const
	{
		return &mOwner->mRenderPasses[mHandle.mIndex];
	}

	void FRenderGraphBuilder::Init()
	{
      TURBO_LOG(LogRenderGraph, Info, "Initialzing RenderGraphBuilder")
      FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

      FDescriptorPoolBuilder descriptorPoolBuilder = {};
      descriptorPoolBuilder
         .SetMaxSets(kMaxDescriptorSets)
         .SetPoolRatio(vk::DescriptorType::eUniformBuffer, 2)
         .SetName(FName("RenderGraphUniforms"));
      mDescriptorPool = gpu.CreateDescriptorPool(descriptorPoolBuilder);
      TURBO_CHECK(mDescriptorPool);

      FDescriptorSetLayoutBuilder descriptroLayoutBuilder = {};
      descriptroLayoutBuilder
         .SetIndex(1)
         .AddBinding(vk::DescriptorType::eUniformBuffer, 0, 1, {}, FName("rgTextureBindingTable"))
         .AddBinding(vk::DescriptorType::eUniformBuffer, 1, 1, {}, FName("rgBufferBindingTable"))
         .SetName(FName("RenderGraphUniforms"));
      mDescriptorSetLayout = gpu.CreateDescriptorSetLayout(descriptroLayoutBuilder);
      TURBO_CHECK(mDescriptorSetLayout)

      /* Create descriptor set per frame in flight */
      for (uint32 frameId = 0; frameId < kMaxFramesInFlight; ++frameId)
      {
			const FName descriptorSetName(fmt::format("RenderGraph_{}", frameId));
			FDescriptorSetBuilder descriptorSetBuilder;
			descriptorSetBuilder
			   .SetDescriptorPool(mDescriptorPool)
			   .SetLayout(mDescriptorSetLayout)
			   .SetName(descriptorSetName);

			mDescriptorSets[frameId] = gpu.CreateDescriptorSet(descriptorSetBuilder);
			TURBO_CHECK(mDescriptorSets[frameId])
      }
	}

	void FRenderGraphBuilder::Shutdown()
	{
   	TURBO_LOG(LogRenderGraph, Info, "Destroying RenderGraphBuilder")

      FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
      gpu.DestroyDescriptorSetLayout(mDescriptorSetLayout);
      gpu.DestroyDescriptorPool(mDescriptorPool);
	}

	FRGResourceHandle FRenderGraphBuilder::CreateTexture(const FRGTextureInfo& textureInfo)
	{
		TURBO_CHECK(textureInfo.IsValid())
		mTextures.push_back(textureInfo);
		return {ERGResourceType::Texture, static_cast<uint32>(mTextures.size() - 1)};
	}

	FRGResourceHandle FRenderGraphBuilder::RegisterExternalTexture(THandle<FTexture> texture, ETextureLayout initLayout)
	{
		return RegisterExternalTexture(texture, initLayout, initLayout);
	}

	FRGResourceHandle FRenderGraphBuilder::RegisterExternalTexture(THandle<FTexture> textureHandle, ETextureLayout initLayout, ETextureLayout finalLayout)
	{
		TURBO_CHECK(textureHandle)

		auto findExternalTexturePredicate = [textureHandle](const FRGTextureInfo& textureInfo)
		{
			return textureInfo.mExternalTextureHandle == textureHandle;
		};

		// Check is texture is registered, if yes return it.
		for (uint32 textureIndex = 0; textureIndex < mTextures.size(); ++textureIndex)
		{
		   if (mTextures[textureIndex].mExternalTextureHandle == textureHandle)
			{
				return {ERGResourceType::Texture, textureIndex, true};
			}
		}

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		const FTexture* texture = gpu.AccessTexture(textureHandle);
		TURBO_CHECK(textureHandle);

		FRGTextureInfo textureInfo = {
			.mWidth = texture->mWidth,
			.mHeight = texture->mHeight,
			.mFormat = texture->GetFormat(),
			.mFlags =  texture->mFlags,

			.mExternalTextureHandle = textureHandle,
			.mInitialLayout = initLayout,
			.mFinalLayout = finalLayout,

			.mName = texture->mName,
		};

		mTextures.push_back(textureInfo);
		return {ERGResourceType::Texture, static_cast<uint32>(mTextures.size()) - 1, true};
	}

	FRGTextureInfo FRenderGraphBuilder::GetTextureInfo(FRGResourceHandle resourceHandle) const
	{
		TURBO_CHECK(resourceHandle.GetType() == ERGResourceType::Texture && resourceHandle.IsValid())
		return mTextures[resourceHandle.GetIndex()];
	}

	FRGResourceHandle FRenderGraphBuilder::CreateBuffer(const FRGBufferInfo& bufferInfo)
	{
		TURBO_CHECK(bufferInfo.IsValid())
		mBuffers.push_back(bufferInfo);
		return {ERGResourceType::Buffer, static_cast<uint32>(mBuffers.size() - 1)};
	}

	void FRenderGraphBuilder::QueueBufferUpload(const FRGBufferUpload& bufferUpload)
	{
#if WITH_SLOW_ASSERTIONS
		const FRGBufferInfo& bufferInfo = mBuffers[bufferUpload.mTargetBuffer.GetIndex()];
		TURBO_CHECK(bufferUpload.mDataSize + bufferUpload.mOffset <= bufferInfo.mSize)
		TURBO_CHECK(mAllocator.Contains(bufferUpload.mData, bufferUpload.mDataSize + bufferUpload.mOffset))
#endif // WITH_SLOW_ASSERTIONS

		mQueuedBufferUploads.push_back(bufferUpload);
	}

	std::tuple<FRGResourceHandle, void* /*intermediatePtr */> FRenderGraphBuilder::CreateAndQueueBufferUpload(const FCreateAndUploadBuffer& createAndUploadBuffer)
	{
		TURBO_CHECK(createAndUploadBuffer.mData)

		FRGResourceHandle result = CreateBuffer(FRGBufferInfo{
			.mSize = createAndUploadBuffer.mSize,
			.mBufferFlags = createAndUploadBuffer.mBufferFlags | EBufferFlags::CreateMapped,
			.mName = createAndUploadBuffer.mName,
		});

		void* data = createAndUploadBuffer.mData;
		if (mAllocator.Contains(createAndUploadBuffer.mData) == false)
		{
			data = mAllocator.Allocate(createAndUploadBuffer.mSize);
			memcpy(data, createAndUploadBuffer.mData, createAndUploadBuffer.mSize);
		}

		QueueBufferUpload(FRGBufferUpload{
			.mTargetBuffer = result,
			.mData = data,
			.mDataSize = createAndUploadBuffer.mSize,
			.mOffset = 0
		});

		return std::make_tuple(result, data);
	}

	FRGBufferInfo FRenderGraphBuilder::GetBufferInfo(FRGResourceHandle resourceHandle) const
	{
		TURBO_CHECK(resourceHandle.GetType() == ERGResourceType::Buffer && resourceHandle.IsValid())
		TURBO_CHECK(resourceHandle.IsExternal() == false)

		return mBuffers[resourceHandle.GetIndex()];
	}

	FRGResourceHandle FRenderGraphBuilder::RegisterExternalBuffer(THandle<FBuffer> bufferHandle)
	{
		TURBO_CHECK(bufferHandle)

		// Check is buffer is registered, if yes return it.
		for (uint32 bufferId = 0; bufferId < mBuffers.size(); ++bufferId)
		{
		   if (mBuffers[bufferId].mExternalBufferHandle == bufferHandle)
			{
				return {ERGResourceType::Buffer, bufferId, true};
			}
		}

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		const FBuffer* buffer = gpu.AccessBuffer(bufferHandle);
		TURBO_CHECK(bufferHandle)

		const FRGBufferInfo externalBufferInfo = {
			.mSize = buffer->mDeviceSize,
			.mBufferFlags = buffer->mBufferFlags,
			.mExternalBufferHandle = bufferHandle,
			.mName = buffer->mName,
		};

		mBuffers.push_back(externalBufferInfo);
		return {ERGResourceType::Buffer, static_cast<uint32>(mBuffers.size()) - 1, true};
	}

	FRGPassInitializer FRenderGraphBuilder::AddPass(FName passName, EPassType passType)
	{
		mRenderPasses.emplace_back();
		FRGPassInfo& passInfo = mRenderPasses.back();
		passInfo.mPassType = passType;
		passInfo.mGraphBuilder = this;
		passInfo.mHandle = { .mIndex = static_cast<uint16>(mRenderPasses.size() - 1) };
		passInfo.mName = passName;

		return FRGPassInitializer(*this, passInfo);
	}

	constexpr vk::AccessFlags2 FindAccessMask(EResourceAccess passType)
	{
		switch (passType)
		{
		case EResourceAccess::Read:
			return vk::AccessFlagBits2::eMemoryRead;
		case EResourceAccess::ReadWrite:
			return vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead;
		default:
			TURBO_UNINPLEMENTED()
		}

		std::unreachable();
	};

	inline vk::PipelineStageFlags2 FindTextureStageMask(EPassType passType, vk::Format format)
	{
		switch (passType)
		{
		case EPassType::Undefined:
			return vk::PipelineStageFlagBits2::eAllCommands;
		case EPassType::Graphics:
			return TextureFormat::HasDepth(format)
				       ? vk::PipelineStageFlagBits2::eEarlyFragmentTests
				       : vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		case EPassType::Compute:
			return vk::PipelineStageFlagBits2::eComputeShader;
		case EPassType::Transfer:
			return vk::PipelineStageFlagBits2::eTransfer;
		default:
			TURBO_UNINPLEMENTED()
		}

		std::unreachable();
	};

	inline vk::PipelineStageFlagBits2 FindBufferStageMask(EPassType passType)
	{
		switch (passType)
		{
		case EPassType::Undefined:
			return vk::PipelineStageFlagBits2::eAllCommands;
		case EPassType::Graphics:
			return vk::PipelineStageFlagBits2::eDrawIndirect;
		case EPassType::Compute:
			return vk::PipelineStageFlagBits2::eComputeShader;
		case EPassType::Transfer:
			return vk::PipelineStageFlagBits2::eTransfer;
		default:
			TURBO_UNINPLEMENTED()
		}

		std::unreachable();
	}

	inline void GroupResourcesByUsage(
		const std::vector<FRGResourceHandle>& inReads,
		const std::vector<FRGResourceHandle>& inWrites,
		std::vector<FRGResourceHandle>& outReadOnly,
		std::vector<FRGResourceHandle>& outReadWrite
	)
	{
		TURBO_CHECK(&inReads != &inWrites && &outReadOnly != &outReadWrite);

		outReadWrite = inWrites;
		for (FRGResourceHandle read : inReads)
		{
			if (std::ranges::find(outReadWrite, read) == outReadWrite.end())
			{
				outReadOnly.push_back(read);
			}
		}
	}

	void FRenderGraphBuilder::CompileTextureSynchronization()
	{
		struct FResourceState
		{
			EPassType mLastUseType = EPassType::Graphics;
			EResourceAccess mAccess = EResourceAccess::ReadWrite;
			ETextureLayout mLayout = ETextureLayout::Undefined;
			vk::Format mFormat = vk::Format::eUndefined;
		};

		entt::dense_map<FRGResourceHandle, FResourceState> resourceData;

		// register external resources
		for (uint32 textureId = 0; textureId < mTextures.size(); ++textureId)
		{
         const FRGTextureInfo& textureInfo = mTextures[textureId];
			if (textureInfo.mExternalTextureHandle.IsValid())
			{
				FRGResourceHandle resourceHandle(ERGResourceType::Texture, textureId, true);
				resourceData[resourceHandle] = {
					.mLastUseType = EPassType::Undefined,
					.mAccess = EResourceAccess::ReadWrite,
					.mLayout = textureInfo.mInitialLayout,
					.mFormat = textureInfo.mFormat,
				};
			}
		}

		mPerPassTextureBarriers.clear();
		mPerPassTextureBarriers.resize(mRenderPasses.size());

		for (uint32 passId = 0; passId < mRenderPasses.size(); ++passId)
		{
			const FRGPassInfo& pass = mRenderPasses[passId];
			TURBO_LOG(LogRenderGraph, Display, "[Compile Texture Syncronization] {}", pass.mName)

			std::vector<FRGTextureMemoryBarrier>& passImageBarriers = mPerPassTextureBarriers[passId];

			std::vector<FRGResourceHandle> readWriteResources = pass.mTextureWrites;
			std::vector<FRGResourceHandle> readOnlyResources;

			GroupResourcesByUsage(
				pass.mTextureReads,
				pass.mTextureWrites,
				readOnlyResources,
				readWriteResources
			);

			for (FRGResourceHandle read : readOnlyResources)
			{
				auto foundSrcData = resourceData.find(read);
				TURBO_CHECK_MSG(foundSrcData != resourceData.end(), "Pass tries to read non existent resource")

				if (FResourceState& srcData = foundSrcData->second;
					srcData.mAccess != EResourceAccess::Read || srcData.mLayout != ETextureLayout::ReadOnly)
				{
					FRGTextureMemoryBarrier& newBarrier = passImageBarriers.emplace_back();
					newBarrier.mTexture = read;
					newBarrier.mOldLayout = srcData.mLayout;
					newBarrier.mNewLayout =
						pass.mPassType == EPassType::Transfer
							? ETextureLayout::TransferSrc
							: ETextureLayout::ReadOnly;

					newBarrier.mSrcStageMask = FindTextureStageMask(srcData.mLastUseType, srcData.mFormat);
					newBarrier.mDstStageMask =
						pass.mPassType == EPassType::Compute
							? vk::PipelineStageFlagBits2::eAllCommands
							: vk::PipelineStageFlagBits2::eAllGraphics;

					newBarrier.mSrcAccessMask = FindAccessMask(srcData.mAccess);
					newBarrier.mDstAccessMask =
						pass.mPassType == EPassType::Transfer
							? vk::AccessFlagBits2::eTransferRead
							: vk::AccessFlagBits2::eShaderRead;

					srcData.mLastUseType = pass.mPassType;
					srcData.mAccess = EResourceAccess::Read;
					srcData.mLayout = ETextureLayout::ReadOnly;

					TURBO_LOG(LogRenderGraph, Display, "[Read Only] {}, {}", GetTextureInfo(read).mName, newBarrier.ToString())
				}
			}

			for (FRGResourceHandle write : readWriteResources)
			{
				FRGTextureMemoryBarrier& newBarrier = passImageBarriers.emplace_back();
				newBarrier.mTexture = write;
				newBarrier.mDstAccessMask = FindAccessMask(EResourceAccess::ReadWrite);

				if (auto srcDataIt = resourceData.find(write);
					srcDataIt != resourceData.end())
				{
					newBarrier.mOldLayout = srcDataIt->second.mLayout;
					newBarrier.mSrcAccessMask = FindAccessMask(srcDataIt->second.mAccess);
					newBarrier.mSrcStageMask = FindTextureStageMask(srcDataIt->second.mLastUseType, srcDataIt->second.mFormat);
				}
				else
				{
					// This pass creates new texture
					newBarrier.mOldLayout = ETextureLayout::Undefined;
					newBarrier.mSrcAccessMask = vk::AccessFlagBits2::eNone;
					newBarrier.mSrcStageMask = vk::PipelineStageFlagBits2::eNone;

					resourceData[write] = {
						.mFormat = GetTextureFormat(write)
					};
				}

				switch (pass.mPassType)
				{
				case EPassType::Graphics:
					newBarrier.mNewLayout =
						pass.mDepthStencilAttachment.mTexture == write
							? ETextureLayout::DepthStencilAttachment
							: ETextureLayout::ColorAttachment;
					break;
				case EPassType::Compute:
					newBarrier.mNewLayout = ETextureLayout::General;
					break;
				case EPassType::Transfer:
					newBarrier.mNewLayout = ETextureLayout::TransferDst;
					break;
				default:
					TURBO_UNINPLEMENTED()
				}

				FResourceState& currentData = resourceData.at(write);
				newBarrier.mDstStageMask = FindTextureStageMask(pass.mPassType, currentData.mFormat);

				TURBO_LOG(LogRenderGraph, Display, "[Read Write] {} {}", GetTextureInfo(write).mName, newBarrier.ToString())


				currentData.mLastUseType = pass.mPassType;
				currentData.mAccess = EResourceAccess::ReadWrite;
				currentData.mLayout = newBarrier.mNewLayout;
			}
		}

		// Add final exterior resources barriers
		for (uint32 textureId = 0; textureId < mTextures.size(); ++textureId)
		{
   		const FRGTextureInfo& textureInfo = mTextures[textureId];
         FRGResourceHandle resourceHandle(ERGResourceType::Texture, textureId, true);

         if (textureInfo.mExternalTextureHandle.IsValid())
         {
   			if (auto foundSrcData = resourceData.find(resourceHandle);
   				foundSrcData != resourceData.end())
   			{
   				FResourceState& srcData = foundSrcData->second;

   				// Skip transitions to undefined
   				if (textureInfo.mFinalLayout != ETextureLayout::Undefined)
   				{
      				FRGTextureMemoryBarrier& newBarrier = mExternalTexturesBarriers.emplace_back();
      				newBarrier.mSrcStageMask = FindTextureStageMask(srcData.mLastUseType, srcData.mFormat);
      				newBarrier.mDstStageMask = vk::PipelineStageFlagBits2::eAllCommands;

      				newBarrier.mSrcAccessMask = FindAccessMask(srcData.mAccess);
      				newBarrier.mDstAccessMask = FindAccessMask(EResourceAccess::ReadWrite);

      				newBarrier.mOldLayout = srcData.mLayout;
      				newBarrier.mNewLayout = textureInfo.mFinalLayout;

      				newBarrier.mTexture = resourceHandle;
   				}
   			}
         }
		}
	}

	void FRenderGraphBuilder::CompileBufferSynchronization()
	{
		struct FResourceState
		{
			EPassType mLastUseType = EPassType::Graphics;
			EResourceAccess mAccess = EResourceAccess::ReadWrite;
		};

		entt::dense_map<FRGResourceHandle, FResourceState> resourceData;

		for (uint32 bufferId = 0; bufferId < mBuffers.size(); ++bufferId)
		{
		   const FRGBufferInfo& bufferInfo = mBuffers[bufferId];
			if (bufferInfo.mExternalBufferHandle.IsValid())
			{
   			FRGResourceHandle resourceHandle(ERGResourceType::Buffer, bufferId, true);
   			resourceData[resourceHandle] = {
   				.mLastUseType = EPassType::Undefined,
   				.mAccess = EResourceAccess::ReadWrite,
   			};

			}
		}

		for (const FRGBufferUpload& upload : mQueuedBufferUploads)
		{
			resourceData[upload.mTargetBuffer] = {
				.mLastUseType = EPassType::Undefined,
				.mAccess = EResourceAccess::HostWrite
			};
		}

		mPerPassBufferBarriers.clear();
		mPerPassBufferBarriers.resize(mRenderPasses.size());

		for (uint32 passId = 0; passId < mRenderPasses.size(); ++passId)
		{
			const FRGPassInfo& pass = mRenderPasses[passId];

			std::vector<FRGResourceHandle> readWriteResources;
			std::vector<FRGResourceHandle> readOnlyResources;

			GroupResourcesByUsage(
				pass.mBufferReads, pass.mBufferWrites,
				readOnlyResources, readWriteResources
			);

			for (FRGResourceHandle read : readOnlyResources)
			{
				auto foundSrcData = resourceData.find(read);
				TURBO_CHECK_MSG(foundSrcData != resourceData.end(), "Pass tries to read non existent resource");

				if (FResourceState& srcData = foundSrcData->second;
					srcData.mAccess != EResourceAccess::Read && srcData.mAccess != EResourceAccess::HostWrite)
				{
					FRGBufferMemoryBarrier& newBarrier = mPerPassBufferBarriers[passId].emplace_back();
					newBarrier.mBuffer = read;

					newBarrier.mSrcAccessMask = FindAccessMask(EResourceAccess::ReadWrite);
					newBarrier.mDstAccessMask = FindAccessMask(EResourceAccess::Read);

					newBarrier.mSrcStageMask = FindBufferStageMask(srcData.mLastUseType);
					newBarrier.mDstStageMask = FindBufferStageMask(pass.mPassType);

					srcData.mLastUseType = pass.mPassType;
					srcData.mAccess = EResourceAccess::Read;
				}
			}

			for (FRGResourceHandle write : readWriteResources)
			{
				auto srcDataIt = resourceData.find(write);
				if (srcDataIt != resourceData.end() && srcDataIt->second.mAccess == EResourceAccess::HostWrite)
				{
					continue;
				}

				FRGBufferMemoryBarrier& newBarrier = mPerPassBufferBarriers[passId].emplace_back();
				newBarrier.mBuffer = write;
				newBarrier.mDstAccessMask = FindAccessMask(EResourceAccess::ReadWrite);

				if (srcDataIt != resourceData.end())
				{
					newBarrier.mSrcAccessMask = FindAccessMask(srcDataIt->second.mAccess);
					newBarrier.mSrcStageMask = FindBufferStageMask(srcDataIt->second.mLastUseType);
				}
				else
				{
					// This pass creates a new buffer
					newBarrier.mSrcAccessMask = vk::AccessFlagBits2::eNone;
					newBarrier.mSrcStageMask = vk::PipelineStageFlagBits2::eNone;

					resourceData[write] = {};
				}

				FResourceState& currentData = resourceData.at(write);
				currentData.mLastUseType = pass.mPassType;
				currentData.mAccess = EResourceAccess::ReadWrite;
			}
		}
	}

	void FRenderGraphBuilder::Compile()
	{
		TRACE_ZONE_SCOPED()

		CompileTextureSynchronization();
		CompileBufferSynchronization();
	}

   void FRenderGraphBuilder::Execute(FGPUDevice& gpu, FCommandBuffer& cmd)
	{
		TRACE_ZONE_SCOPED()
		TURBO_LOG(LogRenderGraph, Display, "Executing render graph");

		const uint32 NumTextures = mTextures.size();
		const uint32 NumBuffers = mBuffers.size();

		FRenderResources renderResources = {};
		renderResources.mTextures.reserve(NumTextures);
		renderResources.mBuffers.reserve(NumBuffers);

		// Allocate textures
		for (uint32 textureId = 0; textureId < mTextures.size(); ++textureId)
		{
			const FRGTextureInfo& textureInfo = mTextures[textureId];

			if (textureInfo.mExternalTextureHandle.IsValid())
			{
   			TURBO_LOG(LogRenderGraph, Display, "Registering external texture: {}", textureInfo.mName);

   			const THandle<FTexture> texture = textureInfo.mExternalTextureHandle;
   			TURBO_CHECK(texture)

   			renderResources.mTextures.push_back(texture);
			}
			else
			{
   			TURBO_LOG(LogRenderGraph, Display, "Allocating texture: {}", textureInfo.mName);

   			FTextureBuilder builder = {
   				.mWidth = textureInfo.mWidth,
   				.mHeight = textureInfo.mHeight,
   				.mFlags = textureInfo.mFlags,
   				.mFormat = textureInfo.mFormat,
   				.mType = ETextureType::Texture2D,
   				.mNumSamples = textureInfo.mNumSamples,
   				.mName = textureInfo.mName
   			};

   			const THandle<FTexture> texture = gpu.CreateTexture(builder);
   			TURBO_CHECK(texture)

   			renderResources.mTextures.push_back(texture);
			}
		}

		// Allocate buffers
		for (uint32 bufferId = 0; bufferId < mBuffers.size(); ++bufferId)
		{
			const FRGBufferInfo bufferInfo = mBuffers[bufferId];

			if (bufferInfo.mExternalBufferHandle.IsValid())
			{
   			TURBO_LOG(LogRenderGraph, Display, "Registering external buffer: {}", bufferInfo.mName);

   			const THandle<FBuffer> buffer = bufferInfo.mExternalBufferHandle;
   			TURBO_CHECK(buffer)

   			renderResources.mBuffers.push_back(buffer);
			}
			else
			{
   			TURBO_LOG(LogRenderGraph, Display, "Allocating buffer: {}", bufferInfo.mName);

   			FBufferBuilder builder = {
   				.mBufferFlags = bufferInfo.mBufferFlags,
   				.mSize = bufferInfo.mSize,
   				.mName = bufferInfo.mName
   			};

   			const FRGResourceHandle handle(ERGResourceType::Buffer, bufferId, false);
   			const THandle<FBuffer> buffer = gpu.CreateBuffer(builder);
   			TURBO_CHECK(buffer)

   			renderResources.mBuffers.push_back(buffer);
			}
		}

		// Upload buffers
		for (FRGBufferUpload& bufferUpload : mQueuedBufferUploads)
		{
   		TURBO_LOG(LogRenderGraph, Display, "Uploading data to \"{}\" buffer", mBuffers[bufferUpload.mTargetBuffer.GetIndex()].mName);

			const FBuffer* buffer = gpu.AccessBuffer(renderResources.GetBuffer(bufferUpload.mTargetBuffer));
			std::memcpy(
				static_cast<byte*>(buffer->mMappedAddress) + bufferUpload.mOffset,
				bufferUpload.mData,
				bufferUpload.mDataSize
			);
		}

		// Create and fill descriptor set
		{
         TRACE_ZONE_SCOPED_N("Create render graph's descriptor set")

		   /* Create buffer address table */
			static const FName bufferAddressTableName("BufferAddressTable");
			const THandle<FBuffer> bufferAddressTableHandle = gpu.CreateBuffer({
				.mBufferFlags = EBufferFlags::UniformBuffer | EBufferFlags::CreateMapped,
				.mSize = kBufferAddressTableSize * sizeof(FDeviceAddress),
				.mName = bufferAddressTableName,
			});
			const FBuffer* bufferAddressTable = gpu.AccessBuffer(bufferAddressTableHandle);
			FDeviceAddress* batMappedAddress = reinterpret_cast<FDeviceAddress*>(bufferAddressTable->mMappedAddress);

			for (uint32 bufferId = 0; bufferId < renderResources.mBuffers.size(); ++bufferId)
			{
   			const FBuffer* buffer = gpu.AccessBuffer(renderResources.mBuffers[bufferId]);
            TURBO_CHECK(buffer)

            batMappedAddress[bufferId] = buffer->mDeviceAddress;
			}

         /* Create texture index table */
         static const FName textureIndexTableName("TextureIndexTable");
         const THandle<FBuffer> textureIndexTableHandle = gpu.CreateBuffer({
            .mBufferFlags = EBufferFlags::UniformBuffer | EBufferFlags::CreateMapped,
            .mSize = kTextureBindingTableSize * sizeof(FHandle::IndexType),
            .mName = textureIndexTableName,
         });
         const FBuffer* textureIndexTable = gpu.AccessBuffer(textureIndexTableHandle);
         FHandle::IndexType* titMappedAddress = reinterpret_cast<FHandle::IndexType*>(textureIndexTable->mMappedAddress);

         for (uint32 textureId = 0; textureId < renderResources.mTextures.size(); ++textureId)
			{
				titMappedAddress[textureId] = renderResources.mTextures[textureId].GetIndex();
			}

			renderResources.mDescriptorSet = mDescriptorSets[gpu.GetFrameInFlightId()];
			FDescriptorSet* descriptorSet = gpu.AccessDescriptorSet(renderResources.mDescriptorSet);

			// TODO: Replace with abstraction (?)
			std::array<vk::WriteDescriptorSet, 2> writeSets;

			vk::WriteDescriptorSet& batWrite = writeSets[0];
			batWrite.dstSet = descriptorSet->mVkDescriptorSet;
			batWrite.descriptorCount = 1;
			batWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
			batWrite.dstBinding = 0;
			vk::DescriptorBufferInfo batBufferInfo = {};
			batBufferInfo.buffer = bufferAddressTable->mVkBuffer;
			batBufferInfo.offset = 0;
			batBufferInfo.range = vk::WholeSize;
			batWrite.pBufferInfo = &batBufferInfo;

			vk::WriteDescriptorSet& titWrite= writeSets[1];
			titWrite.dstSet = descriptorSet->mVkDescriptorSet;
			titWrite.descriptorCount = 1;
			titWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
			titWrite.dstBinding = 1;
			vk::DescriptorBufferInfo titBufferInfo = {};
			titBufferInfo.buffer = textureIndexTable->mVkBuffer;
			titBufferInfo.offset = 0;
			titBufferInfo.range = vk::WholeSize;
			titWrite.pBufferInfo = &titBufferInfo;

			gpu.GetVkDevice().updateDescriptorSets(2, writeSets.data(), 0, nullptr);

			// Destroy buffers at the end of the frame.
			gpu.DestroyBuffer(bufferAddressTableHandle);
			gpu.DestroyBuffer(textureIndexTableHandle);
		}

		for (uint32 passId = 0; passId < mRenderPasses.size(); ++passId)
		{
			const FRGPassInfo& pass = mRenderPasses[passId];
			DEBUG_LABEL_REGION(cmd, pass.mName);

			TURBO_LOG(LogRenderGraph, Display, "Begin render pass: {}", pass.mName);

			// Add a barrier
			FRGPassTextureBarriers& passImageBarriers = mPerPassTextureBarriers[passId];

			std::vector<vk::ImageMemoryBarrier2> imageBarriers;
			imageBarriers.reserve(passImageBarriers.size());

			for (const FRGTextureMemoryBarrier& rgBarrier : passImageBarriers)
			{
				THandle<FTexture> textureHandle = renderResources.GetTexture(rgBarrier.mTexture);
				TURBO_LOG(
					LogRenderGraph, Display, "[Image Barrier] Texture: {}; ({}, {}, {}) -> ({}, {}, {})",
					gpu.AccessTexture(textureHandle)->mName,
					magic_enum::enum_name(rgBarrier.mOldLayout),
					vk::to_string(rgBarrier.mSrcStageMask),
					vk::to_string(rgBarrier.mSrcAccessMask),
					magic_enum::enum_name(rgBarrier.mNewLayout),
					vk::to_string(rgBarrier.mSrcStageMask),
					vk::to_string(rgBarrier.mDstAccessMask)
				);

				imageBarriers.push_back(rgBarrier.ToVkImageBarrier(gpu, textureHandle));
			}

			FRGPassBufferBarriers& passBufferBarriers = mPerPassBufferBarriers[passId];

			std::vector<vk::BufferMemoryBarrier2> bufferBarriers;
			bufferBarriers.reserve(mPerPassBufferBarriers[passId].size());

			for (const FRGBufferMemoryBarrier& rgBarrier : passBufferBarriers)
			{
				THandle<FBuffer> bufferHandle = renderResources.GetBuffer(rgBarrier.mBuffer);
				bufferBarriers.push_back(rgBarrier.ToVkBufferBarrier(gpu, bufferHandle));

				TURBO_LOG(LogRenderGraph, Display, "[Buffer Barrier] Buffer: {}", gpu.AccessBuffer(bufferHandle)->mName);
			}

			vk::DependencyInfo dependencyInfo = {};
			dependencyInfo.imageMemoryBarrierCount = imageBarriers.size();
			dependencyInfo.pImageMemoryBarriers = imageBarriers.data();
			dependencyInfo.bufferMemoryBarrierCount = bufferBarriers.size();
			dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();

			cmd.PipelineBarrier(dependencyInfo);

			if (pass.mPassType == EPassType::Graphics)
			{
				FRenderingAttachments renderingAttachments;

				// Bind color attachments
				for (uint32 attachmentId = 0; attachmentId < kMaxColorAttachments; ++attachmentId)
				{
					if (pass.mColorAttachments[attachmentId].IsValid())
					{
						const FRGAttachment attachment = pass.mColorAttachments[attachmentId];

						FAttachment attachmentInfo = {
							.mTexture = renderResources.GetTexture(attachment.mTexture),
							.mLoadOp = attachment.mLoadOp,
							.mStoreOp = attachment.mStoreOp,
							.mClearColor = attachment.mClearColor,
						};

						if (attachment.mResolveTexture.IsValid())
						{
							attachmentInfo.mResolveTexture = renderResources.GetTexture(attachment.mResolveTexture),
							attachmentInfo.mResolveMode = attachment.mResolveMode;
						}

						renderingAttachments.AddColorAttachment(attachmentInfo);

						TURBO_LOG(
							LogRenderGraph, Display, "[GraphicsPass] Bind {} as color attachment {}",
							gpu.AccessTexture(renderResources.GetTexture(attachment.mTexture))->mName,
							attachmentId
						);
					}
				}

				// Bind depth stencil attachment (if valid)
				if (pass.mDepthStencilAttachment.IsValid())
				{
					const FRGAttachment& attachment = pass.mDepthStencilAttachment;

					FAttachment attachmentInfo = {
						.mTexture = renderResources.GetTexture(attachment.mTexture),
						.mLoadOp = attachment.mLoadOp,
						.mStoreOp = attachment.mStoreOp,
						.mClearColor = attachment.mClearColor,
					};

					if (attachment.mResolveTexture.IsValid())
					{
						attachmentInfo.mResolveTexture = renderResources.GetTexture(attachment.mResolveTexture),
						attachmentInfo.mResolveMode = attachment.mResolveMode;
					}

					renderingAttachments.SetDepthAttachment(attachmentInfo);

					TURBO_LOG(
						LogRenderGraph, Display, "[GraphicsPass] Bind {} as depth attachment",
						gpu.AccessTexture(renderResources.GetTexture(pass.mDepthStencilAttachment.mTexture))->mName
					);
				}

				const FRGResourceHandle mainTextureHandle =
					pass.mColorAttachments[0].IsValid()
						? pass.mColorAttachments[0].mTexture
						: pass.mDepthStencilAttachment.mTexture;

				TURBO_CHECK(mainTextureHandle.IsValid())

				const FRGTextureInfo& textureInfo = mTextures[mainTextureHandle.GetIndex()];
				const glm::ivec2 outputSize = glm::ivec2(textureInfo.mWidth, textureInfo.mHeight);

				cmd.BeginRendering(renderingAttachments);
				cmd.SetViewport(FViewport::FromSize(outputSize));
				cmd.SetScissor(FRect2DInt::FromSize(outputSize));

				TURBO_LOG(LogRenderGraph, Display, "[GraphicsPass] Begin rendering. Viewport: {} Scissors: {}", outputSize, outputSize);
			}

			TURBO_CHECK(pass.mExecutePass.IsBound());

			TURBO_LOG(LogRenderGraph, Display, "Execute: {}", pass.mName);
			pass.mExecutePass.Execute(gpu, cmd, renderResources);

			if (pass.mPassType == EPassType::Graphics)
			{
				TURBO_LOG(LogRenderGraph, Display, "[GraphicsPass] End rendering.");
				cmd.EndRendering();
			}

		}

		// Destroy resources
		// Destroy textures
		for (uint32 textureId = 0; textureId < mTextures.size(); ++textureId)
		{
         const FRGTextureInfo& textureInfo = mTextures[textureId];
         if (textureInfo.mExternalTextureHandle.IsValid() == false)
         {
            THandle<FTexture> textureHandle = renderResources.mTextures[textureId];
            TURBO_LOG(LogRenderGraph, Display, "Destroying texture: {}", gpu.AccessTexture(textureHandle)->mName);
   			gpu.DestroyTexture(textureHandle);
         }
		}

		// Destroy buffers
		for (uint32 bufferId = 0; bufferId < mBuffers.size(); ++bufferId)
		{
		   const FRGBufferInfo& bufferInfo = mBuffers[bufferId];
			if (bufferInfo.mExternalBufferHandle.IsValid() == false)
			{
            THandle<FBuffer> bufferHandle = renderResources.mBuffers[bufferId];
            TURBO_LOG(LogRenderGraph, Display, "Destroying buffer: {}", gpu.AccessBuffer(bufferHandle)->mName);

            gpu.DestroyBuffer(bufferHandle);
			}
		}

		TURBO_LOG(LogRenderGraph, Display, "Final external resources barrier.");

		// Transition external textures to their target layouts
		std::vector<vk::ImageMemoryBarrier2> imageBarriers;
		imageBarriers.reserve(mExternalTexturesBarriers.size());

		for (const FRGTextureMemoryBarrier& rgBarrier : mExternalTexturesBarriers)
		{
			const THandle<FTexture> textureHandle = renderResources.GetTexture(rgBarrier.mTexture);
			imageBarriers.push_back(rgBarrier.ToVkImageBarrier(gpu, textureHandle));

			TURBO_LOG(
				LogRenderGraph, Display, "[External Image Barrier] Texture: {}; ({}, {}, {}) -> ({}, {}, {})",
				gpu.AccessTexture(textureHandle)->mName,
				magic_enum::enum_name(rgBarrier.mOldLayout),
				vk::to_string(rgBarrier.mSrcStageMask),
				vk::to_string(rgBarrier.mSrcAccessMask),
				magic_enum::enum_name(rgBarrier.mNewLayout),
				vk::to_string(rgBarrier.mSrcStageMask),
				vk::to_string(rgBarrier.mDstAccessMask)
			);
		}

		vk::DependencyInfo dependencyInfo = {};
		dependencyInfo.imageMemoryBarrierCount = imageBarriers.size();
		dependencyInfo.pImageMemoryBarriers = imageBarriers.data();

		cmd.PipelineBarrier(dependencyInfo);
	}

	void FRenderGraphBuilder::Reset()
	{
		mRenderPasses.clear();
		mPerPassTextureBarriers.clear();
		mExternalTexturesBarriers.clear();

		mTextures.clear();
		mBuffers.clear();
		mQueuedBufferUploads.clear();

		mAllocator.Clear();
	}

	vk::Format FRenderGraphBuilder::GetTextureFormat(FRGResourceHandle resourceHandle) const
	{
		TURBO_CHECK(resourceHandle.GetType() == ERGResourceType::Texture)
		return mTextures[resourceHandle.GetIndex()].mFormat;
	}
} // Turbo
