#pragma once

#include "Core/DataStructures/Handle.h"
#include "DestoryQueue.h"
#include "Enums.h"
#include "Graphics/GraphicsCore.h"

#define DESTROYER_BODY()				\
	public:								\
		friend class FGPUDevice;

namespace Turbo
{
	struct FPipelineBuilder;

	enum class EResourceType : u8
	{
		Texture,
		RWTexture,
		Sampler,
		TLAS,

		None,
	};

	namespace BindlessResourcesBindings
	{
		constexpr u32 kSampledImage = 0;
		constexpr u32 kStorageImage = 1;
		constexpr u32 kSampler = 2;
		constexpr u32 kTLAS = 3;
	}

	struct FBindlessResourceUpdateRequest
	{
		EResourceType mType = EResourceType::None;
		u32 mBindingIndex = std::numeric_limits<u32>::max();
		FHandle mHandle = FHandle();
	};

	/* Vulkan object abstractions */

	struct FBuffer
	{
		vk::Buffer mVkBuffer = nullptr;

		FDeviceSize mDeviceSize = {};
		FDeviceAddress mDeviceAddress = {};
		ByteType* mMappedAddress = nullptr;

		vma::Allocation mAllocation = nullptr;
		EBufferFlags mBufferFlags = EBufferFlags::None;

		THandle<FBuffer> mHandle = {};
		FName mName;

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const {return IsValid();}
	};

	class FBufferDestroyer : IDestroyer
	{
		DESTROYER_BODY()
	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Buffer mVkBuffer = nullptr;
		vma::Allocation mAllocation = nullptr;
		THandle<FBuffer> mHandle;
	};

	struct FSampler
	{
		vk::Sampler mVkSampler = nullptr;

		vk::Filter mMinFilter = vk::Filter::eNearest;
		vk::Filter mMagFilter = vk::Filter::eNearest;
		vk::SamplerMipmapMode mMipFilter = vk::SamplerMipmapMode::eNearest;

		vk::SamplerAddressMode mAddressModeU = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeV = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeW = vk::SamplerAddressMode::eRepeat;

		THandle<FSampler> mHandle;
		FName mName = {};

		u32 _PAD;

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const {return IsValid();}
	};

	class FSamplerDestroyer : IDestroyer
	{
		DESTROYER_BODY()
	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Sampler mVkSampler;
		THandle<FSampler> mHandle;
	};

	struct FTexture
	{
		vk::Image mVkImage = nullptr;
		vk::ImageView mVkImageView = nullptr;
		vma::Allocation mImageAllocation = nullptr;
		u32 mBindIndex = std::numeric_limits<u32>::max();

		ETextureFlags mFlags = ETextureFlags::Invalid;

		vk::Format mFormat = vk::Format::eUndefined;

		u16 mWidth = 1;
		u16 mHeight = 1;
		u16 mDepth = 1;
		u8 mNumMips = 1;

		THandle<FTexture> mHandle = {};
		FName mName = {};

		[[nodiscard]] glm::int2 GetSize2D() const { return glm::ivec2{mWidth, mHeight}; }
		[[nodiscard]] glm::int3 GetSize() const { return glm::ivec3{mWidth, mHeight, mDepth}; }
		[[nodiscard]] vk::Format GetFormat() const { return mFormat; }

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const {return IsValid();}
	};

	class FTextureDestroyer : public IDestroyer
	{
		DESTROYER_BODY()
	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Image mImage = nullptr;
		vk::ImageView mImageView = nullptr;
		vma::Allocation mImageAllocation = nullptr;
		THandle<FTexture> mHandle = {};
	};

	struct FShaderState
	{
		std::array<vk::PipelineShaderStageCreateInfo, kMaxShaderStages> mShaderStageCrateInfo;

		u32 mNumActiveShaders = 0;
		bool mbGraphicsPipeline = true;

		THandle<FShaderState> mHandle;
		FName mName;

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const {return IsValid();}
	};

	class FShaderStateDestroyer : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		std::array<vk::ShaderModule, kMaxShaderStages> mModules;
		u32 mNumActiveShaders = 0;

		THandle<FShaderState> mHandle;
	};

	struct FBinding
	{
		vk::DescriptorType mType = {};
		u16 mIndex = 0;
		u16 mCount = 0;
		vk::DescriptorBindingFlags mFlags = {};

		FName mName;
	};

	struct FDescriptorSetLayout
	{
		vk::DescriptorSetLayout mVkLayout = nullptr;

		u16 mNumBindings = 0;
		u16 mSetIndex = 0;

		THandle<FDescriptorSetLayout> mHandle = {};

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const {return IsValid();}
	};

	struct FDescriptorPool;

	struct FDescriptorSet
	{
		vk::DescriptorSet mVkDescriptorSet = nullptr;

		THandle<FDescriptorSet> mHandle;
		FName mName;

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const {return IsValid();}
	};

	struct FDescriptorPool
	{
		[[nodiscard]] vk::DescriptorPool GetDescriptorPool() const { return mVkDescriptorPool; }

		vk::DescriptorPool mVkDescriptorPool;
		std::vector<THandle<FDescriptorSet>> mDescriptorSets;

		THandle<FDescriptorPool> mHandle;
		FName mName;

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const {return IsValid();}
	};

	class FDescriptorSetLayoutDestroyer : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::DescriptorSetLayout mVkLayout = nullptr;
		THandle<FDescriptorSetLayout> mHandle = {};
	};

	class FDescriptorPoolDestroyer : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::DescriptorPool mVkDescriptorPool;
		THandle<FDescriptorPool> mhandle;
	};

	struct FPipeline
	{
		vk::Pipeline mVkPipeline = nullptr;
		vk::PipelineLayout mVkLayout = nullptr;

		vk::PipelineBindPoint mVkBindPoint = {};

		THandle<FShaderState> mShaderState = {};
		FPipelineBuilder* mPipelineBuilder = nullptr; // Allows to recompile pipeline at runtime. Replace with handle?

		bool mbGraphicsPipeline = true;

		THandle<FPipeline> mHandle;
		FName mName;

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const {return IsValid();}
	};

	class FPipelineDestroyer : IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Pipeline mPipeline = nullptr;
		vk::PipelineLayout mLayout = nullptr;
		THandle<FPipeline> mHandle = {};
	};

	enum class EAccelerationStructureType
	{
	   BLAS,
		TLAS
	};

	struct FAccelerationStructure
	{
		vk::AccelerationStructureKHR mVkAccelerationStructure;
		FDeviceAddress mDeviceAddress;
		THandle<FBuffer> mBuffer;
		EAccelerationStructureType mType = EAccelerationStructureType::BLAS;

		FHandle mHandle;
		FName mName = {};

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const {return IsValid();}
	};

	struct FBLAS : FAccelerationStructure
	{
	};
	static_assert(sizeof(FBLAS) == sizeof(FAccelerationStructure));

	struct FTLAS : FAccelerationStructure
	{
	};
	static_assert(sizeof(FTLAS) == sizeof(FAccelerationStructure));

	class FAccelerationStructureDestroyer : IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	protected:
		vk::AccelerationStructureKHR mAccelerationStructure;
		THandle<FBuffer> mBuffer;
		FHandle mHandle;
		EAccelerationStructureType mType;
	};

	/** Vulkan object abstractions end */

} // Turbo

#undef RESOURCE_BODY
