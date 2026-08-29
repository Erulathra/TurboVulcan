#pragma once

#include "Core/DataStructures/Handle.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/Resources.h"

namespace Turbo
{
	class GPUDevice;
	struct FTexture;

	struct FRGPassHandle
	{
		[[nodiscard]] bool IsValid() const { return mIndex != std::numeric_limits<u16>::max(); }

		u32 mIndex = std::numeric_limits<u16>::max();

		friend bool operator==(const FRGPassHandle& lhs, const FRGPassHandle& rhs)
		{
			return lhs.mIndex == rhs.mIndex;
		}

		friend bool operator!=(const FRGPassHandle& lhs, const FRGPassHandle& rhs)
		{
			return !(lhs == rhs);
		}
	};

	enum class ERGResourceType : u8
	{
		Invalid,
		Texture,
		Buffer
	};

	struct FRGResourceHandle
	{
   	/* Constants */
		static constexpr u32 kTypeMask = 0xE0000000;
		static constexpr u32 kExternalMax = 0x10000000;
		static constexpr u32 kIndexMask = 0x0FFFFFFF;
		static constexpr u32 kInvalidHandle = 0xFFFFFFFF;

		static_assert(kTypeMask | kExternalMax | kIndexMask == 0xFFFFFFFF);

		/* Data */
		u32 mHandle = kInvalidHandle;

		/* Interface */
		FRGResourceHandle() = default;
		FRGResourceHandle(ERGResourceType type, u32 index, bool bExternal = false)
		{
			mHandle = static_cast<u32>(type) << std::countr_zero(kTypeMask)
				| (bExternal ? 1u : 0u) << std::countr_zero(kExternalMax)
				| index << std::countr_zero(kIndexMask);
		}

		[[nodiscard]] bool IsValid() const { return mHandle != kInvalidHandle; }
		[[nodiscard]] ERGResourceType GetType() const
		{
			return static_cast<ERGResourceType>((mHandle & kTypeMask) >> std::countr_zero(kTypeMask));
		}

		[[nodiscard]] bool IsExternal() const { return (mHandle & kExternalMax) != 0; }
		[[nodiscard]] u32 GetIndex() const { return (mHandle & kIndexMask) >> std::countr_zero(kIndexMask); }

		/* Operators */
		friend bool operator==(const FRGResourceHandle& lhs, const FRGResourceHandle& rhs)
		{
			return lhs.mHandle == rhs.mHandle;
		}

		friend bool operator!=(const FRGResourceHandle& lhs, const FRGResourceHandle& rhs)
		{
			return !(lhs == rhs);
		}

		FRGResourceHandle(const FRGResourceHandle& other) = default;
		FRGResourceHandle& operator=(const FRGResourceHandle& other) = default;
	};
}

template <>
struct std::hash<Turbo::FRGResourceHandle>
{
	size_t operator()(Turbo::FRGResourceHandle handle) const noexcept
	{
		return static_cast<u64>(handle.mHandle) << 32;
	}
};

namespace Turbo
{
	enum class EPassType
	{
		Undefined,
		Graphics,
		Compute,
		Transfer
	};

	enum class ETextureLayout
	{
		Undefined,
		General,
		ReadOnly,

		ColorAttachment,
		DepthStencilAttachment,

		TransferSrc,
		TransferDst,

		PresentSrc
	};

	constexpr vk::ImageLayout ToVkImageLayout(ETextureLayout layout)
	{
		switch (layout)
		{
		case ETextureLayout::Undefined:
			return vk::ImageLayout::eUndefined;
		case ETextureLayout::General:
			return vk::ImageLayout::eGeneral;
		case ETextureLayout::ReadOnly:
			return vk::ImageLayout::eReadOnlyOptimal;
		case ETextureLayout::ColorAttachment:
			return vk::ImageLayout::eColorAttachmentOptimal;
		case ETextureLayout::DepthStencilAttachment:
			return vk::ImageLayout::eDepthStencilAttachmentOptimal;
		case ETextureLayout::TransferSrc:
			return vk::ImageLayout::eTransferSrcOptimal;
		case ETextureLayout::TransferDst:
			return vk::ImageLayout::eTransferDstOptimal;
		case ETextureLayout::PresentSrc:
			return vk::ImageLayout::ePresentSrcKHR;
		InvalidDefaultCase;
		}
	}

	constexpr ETextureLayout FromVkImageLayout(vk::ImageLayout layout)
	{
		switch (layout)
		{
		case vk::ImageLayout::eUndefined:
			return ETextureLayout::Undefined;
		case vk::ImageLayout::eGeneral:
			return ETextureLayout::General;
		case vk::ImageLayout::eReadOnlyOptimal:
			return ETextureLayout::ReadOnly;
		case vk::ImageLayout::eColorAttachmentOptimal:
			return ETextureLayout::ColorAttachment;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			return ETextureLayout::DepthStencilAttachment;
		case vk::ImageLayout::eTransferSrcOptimal:
			return ETextureLayout::TransferSrc;
		case vk::ImageLayout::eTransferDstOptimal:
			return ETextureLayout::TransferDst;
		case vk::ImageLayout::ePresentSrcKHR:
			return ETextureLayout::PresentSrc;
		InvalidDefaultCase;
		}
	}

	enum class EResourceAccess
	{
		Read,
		ReadWrite,

		HostWrite,
	};

	struct FRGTextureInfo
	{
		u16 mWidth = 1;
		u16 mHeight = 1;
		u16 mDepth = 1;

		vk::Format mFormat = vk::Format::eUndefined;
		ETextureFlags mFlags = ETextureFlags::Invalid;
		EMSAASamples mNumSamples = EMSAASamples::One;

		/* If valid, the texture is external */
		THandle<FTexture> mExternalTextureHandle = {};

		/* Valid only if the texture is external */
		ETextureLayout mInitialLayout = ETextureLayout::Undefined;
		ETextureLayout mFinalLayout = ETextureLayout::Undefined;

		FName mName = {};

		[[nodiscard]] bool IsValid() const;
	};

	struct FRGBufferInfo
	{
		FDeviceSize mSize = 0;
		EBufferFlags mBufferFlags = {};

		/* If valid, the buffer is external */
		THandle<FBuffer> mExternalBufferHandle = {};

		FName mName = {};

		[[nodiscard]] bool IsValid() const;
	};

	struct FRGBufferMemoryBarrier
	{
		vk::PipelineStageFlags2 mSrcStageMask = vk::PipelineStageFlagBits2::eAllCommands;
		vk::PipelineStageFlags2 mDstStageMask = vk::PipelineStageFlagBits2::eAllCommands;

		vk::AccessFlags2 mSrcAccessMask = vk::AccessFlagBits2::eMemoryWrite;
		vk::AccessFlags2 mDstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead;

		FRGResourceHandle mBuffer = {};

		[[nodiscard]] vk::BufferMemoryBarrier2 ToVkBufferBarrier(GPUDevice* gpu, THandle<FBuffer> bufferHandle) const;
	};

	struct FRGTextureMemoryBarrier
	{
		ETextureLayout mOldLayout = ETextureLayout::Undefined;
		ETextureLayout mNewLayout = ETextureLayout::Undefined;

		vk::PipelineStageFlags2 mSrcStageMask = vk::PipelineStageFlagBits2::eAllCommands;
		vk::PipelineStageFlags2 mDstStageMask = vk::PipelineStageFlagBits2::eAllCommands;

		vk::AccessFlags2 mSrcAccessMask = vk::AccessFlagBits2::eMemoryWrite;
		vk::AccessFlags2 mDstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead;

		FRGResourceHandle mTexture = {};

		[[nodiscard]] vk::ImageMemoryBarrier2 ToVkImageBarrier(GPUDevice* gpu, THandle<FTexture> textureHandle) const;
		[[nodiscard]] std::string ToString() const;
	};

	struct FRGResourceLifetime
	{
		u16 mFirstPass = U16_MAX;
		u16 mLastPass = 0;
	};

	struct FRGBufferUpload
	{
		FRGResourceHandle mTargetBuffer = {};
		void* mData = nullptr;
		size_t mDataSize = 0;
		size_t mOffset = 0;
	};

	struct FCreateAndUploadBuffer
	{
		void* mData = nullptr;
		FDeviceSize mSize = 0;
		EBufferFlags mBufferFlags;
		FName mName = {};
	};

	struct FRGAttachment
	{
		FRGResourceHandle mTexture = {};
		ELoadOp mLoadOp = ELoadOp::Load;
		EStoreOp mStoreOp = EStoreOp::Store;
		EClearColor mClearColor = EClearColor::Zero;

		FRGResourceHandle mResolveTexture = {};
		EResolveMode mResolveMode = EResolveMode::Average;

		bool IsValid() const { return mTexture.GetType() == ERGResourceType::Texture && mTexture.IsValid(); }
	};
}
