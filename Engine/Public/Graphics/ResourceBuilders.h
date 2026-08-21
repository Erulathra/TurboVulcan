#pragma once

#include "CommonTypeDefs.h"
#include "Graphics/CommandBuffer.h"
#include "Resources.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/Enums.h"
#include "Graphics/VulkanHelpers.h"

namespace Turbo
{
	class FWindow;

	struct FGPUDeviceBuilder
	{
	};

	struct FBufferBuilder
	{
		static FBufferBuilder CreateStagingBuffer(const void* data, u32 size);
		static FBufferBuilder CreateStagingBuffer(u32 size);
		static FBufferBuilder CreateStagingBuffer(std::span<ByteType> data);
		static FBufferBuilder CreateScratchBuffer(u32 size);

		FBufferBuilder& Reset() { mSize = 0; mInitialData = nullptr; return *this; }
		FBufferBuilder& Init(EBufferFlags bufferFlags, size_t size)
		{
			mBufferFlags = bufferFlags;
			mSize = size;
			return *this;
		}
		FBufferBuilder& SetData(const void* data) { mInitialData = data; return *this; }
		FBufferBuilder& SetName(FName name) { mName = name; return *this; }

	public:
		EBufferFlags mBufferFlags = EBufferFlags::None;
		size_t mSize = 0;

		const void* mInitialData = nullptr;

		FName mName;
	};

	enum class EDummyTextureType
	{
		Black,
		White,
		Normal
	};

	struct FTextureBuilder
	{
	public:
		FTextureBuilder& Init(vk::Format format, ETextureType type, ETextureFlags flags = ETextureFlags::Default)
			{ mFormat = format; mType = type; mFlags = flags;  return *this; }
		FTextureBuilder& SetSize(glm::uint3 size) { mWidth = size.x; mHeight = size.y; mDepth = size.z; return *this; }
		FTextureBuilder& SetNumSamples(EMSAASamples numSamples) { mNumSamples = numSamples; return *this; }
		FTextureBuilder& SetNumMips(u8 numMips) { mNumMips = numMips; return *this; }
		FTextureBuilder& SetBindTexture(bool bBindTexture) { mbBindTexture = bBindTexture; return *this; }

		FTextureBuilder& SetName(FName name) { mName = name; return *this; }

	public:
		u16 mWidth = 1;
		u16 mHeight = 1;
		u16 mDepth = 1;
		u8 mNumMips = 1;
		ETextureFlags mFlags = ETextureFlags::Invalid;

		vk::Format mFormat = vk::Format::eUndefined;
		ETextureType mType = ETextureType::Texture2D;

		EMSAASamples mNumSamples = EMSAASamples::One;

		bool mbBindTexture = true;

		FName mName;
	};

	struct FSamplerBuilder
	{
	public:
		FSamplerBuilder& SetMinMagFilter(vk::Filter min, vk::Filter mag) { mMinFilter = min; mMagFilter = mag; return *this; }
		FSamplerBuilder& SetMipFilter(vk::SamplerMipmapMode filter) { mMipFilter = filter; return *this; }

		FSamplerBuilder& SetAddressU(vk::SamplerAddressMode u) { mAddressModeU = u; return *this; }
		FSamplerBuilder& SetAddressUV(vk::SamplerAddressMode u, vk::SamplerAddressMode v) { mAddressModeU = u; mAddressModeV = v; return *this; }
		FSamplerBuilder& SetAddressUVW(vk::SamplerAddressMode u, vk::SamplerAddressMode v, vk::SamplerAddressMode w)
			{ mAddressModeU = u; mAddressModeV = v; mAddressModeV = w; return *this; }
		FSamplerBuilder& SetAddress(vk::SamplerAddressMode mode)
			{ mAddressModeU = mode; mAddressModeV = mode; mAddressModeV = mode; return *this; }

		FSamplerBuilder& SetName(FName name) { mName = name; return *this; }

	public:
		vk::Filter mMinFilter = vk::Filter::eNearest;
		vk::Filter mMagFilter = vk::Filter::eNearest;
		vk::SamplerMipmapMode mMipFilter = vk::SamplerMipmapMode::eNearest;

		vk::SamplerAddressMode mAddressModeU = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeV = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeW = vk::SamplerAddressMode::eRepeat;

		FName mName;
	};

	struct FDescriptorPoolBuilder
	{
	public:
		FDescriptorPoolBuilder();
		FDescriptorPoolBuilder& Reset();
		FDescriptorPoolBuilder& SetPoolRatio(vk::DescriptorType type, fp32 ratio);

		FDescriptorPoolBuilder& SetFlags(vk::DescriptorPoolCreateFlags flags) {mFlags = flags; return *this;}
		FDescriptorPoolBuilder& SetMaxSets(u32 maxSets) { mMaxSets = maxSets; return *this; }
		FDescriptorPoolBuilder& SetName(FName name) { mName = name; return *this; }

	public:
		std::unordered_map<vk::DescriptorType, fp32 /** Ratio **/> mPoolSizes = {};
		u32 mMaxSets = 0;

		vk::DescriptorPoolCreateFlags mFlags;

		FName mName;
	};

	struct FDescriptorSetLayoutBuilder
	{
	public:
		FDescriptorSetLayoutBuilder& Reset() { *this = {}; return *this; }
		FDescriptorSetLayoutBuilder& AddBinding(vk::DescriptorType type, u16 start, u16 count, vk::DescriptorBindingFlags flags = {}, FName name = FName())
		{
			TURBO_CHECK_MSG(count > 0, "Binding count must be greater than 0.")

			mBindings[mNumBindings] = {type, start, count, flags, name};
			++mNumBindings;

			return *this;
		}
		FDescriptorSetLayoutBuilder& AddBinding(vk::DescriptorType type, u16 id, vk::DescriptorBindingFlags flags = {}, FName name = FName())
		{
			return AddBinding(type, id, 1, flags, name);
		}
		FDescriptorSetLayoutBuilder& SetIndex(u16 index) { mSetIndex = index; return *this; }
		FDescriptorSetLayoutBuilder& SetFlags(vk::DescriptorSetLayoutCreateFlags flags) { mFlags = flags; return *this; }

		FDescriptorSetLayoutBuilder& SetName(FName name) { mName = name; return *this; }

	public:
		std::array<FBinding, kMaxDescriptorsPerSet> mBindings;
		u16 mNumBindings = 0;
		u16 mSetIndex = 0;
		vk::DescriptorSetLayoutCreateFlags mFlags = {};

		FName mName;
	};

	struct FDescriptorSetBuilder
	{
	public:
		FDescriptorSetBuilder& SetLayout(THandle<FDescriptorSetLayout> layout) { mLayout = layout; return *this; }
		FDescriptorSetBuilder& SetDescriptorPool(THandle<FDescriptorPool> descriptorPool) { mDescriptorPool = descriptorPool; return *this; }

		FDescriptorSetBuilder& SetName(FName name) { mName = name; return *this; }
		FDescriptorSetBuilder& SetFlags(vk::DescriptorPoolCreateFlags flags) { mFlags = flags; return *this; }

	public:
		THandle<FDescriptorSetLayout> mLayout = {};
		THandle<FDescriptorPool> mDescriptorPool = {};

		vk::DescriptorPoolCreateFlags mFlags;

		FName mName;
	};

	struct FRasterizationBuilder
	{
	public:
		FRasterizationBuilder& SetCullMode(vk::CullModeFlags cullMode) { mCullMode = cullMode; return *this; }
		FRasterizationBuilder& SetPolygonMode(vk::CullModeFlags polygonMode) { mCullMode = polygonMode; return *this; }
		FRasterizationBuilder& SetFrontFace(vk::FrontFace frontFace) { mFrontFace = frontFace; return *this; }

	public:
		vk::CullModeFlags mCullMode = vk::CullModeFlagBits::eBack;
		vk::PolygonMode mPolygonMode = vk::PolygonMode::eFill;
		vk::FrontFace mFrontFace = vk::FrontFace::eCounterClockwise;
	};

	struct FDepthStencilBuilder
	{
	public:
		FDepthStencilBuilder& SetDepth(bool bTest, bool bWrite, vk::CompareOp compareOperator)
		{
			mbEnableDepthTest = bTest;
			mbEnableWriteDepth = bWrite;
			mDepthCompareOperator = compareOperator;

			return *this;
		}

	public:
		vk::CompareOp mDepthCompareOperator = vk::CompareOp::eGreater;

		bool mbEnableDepthTest : 1 = false;
		bool mbEnableWriteDepth : 1 = false;
		bool mbEnableStencil : 1 = false;
		u8 mPadding : 5 = 0;
	};

	struct FBlendState
	{
	public:
		FBlendState& Init(vk::BlendFactor source, vk::BlendFactor destination, vk::BlendOp blendOperator)
		{
			InitColor(source, destination, blendOperator);
			InitAlpha(source, destination, blendOperator);

			return *this;
		}

		FBlendState& InitColor(vk::BlendFactor source, vk::BlendFactor destination, vk::BlendOp blendOperator)
		{
			mbBlendEnabled = true;

			mSourceColorBlendFactor = source;
			mDestinationColorBlendFactor = destination;
			mColorBlendOperator = blendOperator;

			return *this;
		}

		FBlendState& InitAlpha(vk::BlendFactor source, vk::BlendFactor destination, vk::BlendOp blendOperator)
		{
			mbBlendEnabled = true;

			mSourceAlphaBlendFactor = source;
			mDestinationAlphaBlendFactor = destination;
			mAlphaBlendOperator = blendOperator;

			return *this;
		}

		FBlendState& SetWriteMask(vk::ColorComponentFlags writeMask) { mColorComponentMask = writeMask; return *this; }

		FBlendState& InitAlphaBlending()
		{
			InitColor(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd);
			InitColor(vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd);

			return *this;
		}

		FBlendState& InitAdditiveBlending()
		{
			InitColor(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOne, vk::BlendOp::eAdd);
			InitColor(vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd);

			return *this;
		}

	public:
		vk::BlendFactor mSourceColorBlendFactor = vk::BlendFactor::eOne;
		vk::BlendFactor mDestinationColorBlendFactor = vk::BlendFactor::eOne;
		vk::BlendOp mColorBlendOperator = vk::BlendOp::eAdd;

		vk::BlendFactor mSourceAlphaBlendFactor = vk::BlendFactor::eOne;
		vk::BlendFactor mDestinationAlphaBlendFactor = vk::BlendFactor::eOne;
		vk::BlendOp mAlphaBlendOperator = vk::BlendOp::eAdd;

		vk::ColorComponentFlags mColorComponentMask = vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags;

		bool mbBlendEnabled = false;
	};

	struct FBlendStateBuilder
	{
	public:
		FBlendStateBuilder& Reset() { mActiveStates = 0; return *this; }
		FBlendState& AddBlendState()
		{
			mBlendStates[mActiveStates] = FBlendState();
			++mActiveStates;

			return mBlendStates[mActiveStates - 1];
		}

		FBlendStateBuilder& AddNoBlendingState()
		{
			AddBlendState();
			return *this;
		}

	public:
		std::array<FBlendState, kMaxColorAttachments> mBlendStates;
		u32 mActiveStates = 0;
	};

	struct FShaderStage
	{
		std::string mShaderName;
		vk::ShaderStageFlagBits mStage;
		std::string mEntryPoint;
	};

	struct FPipelineRenderingBuilder
	{
	public:
		FPipelineRenderingBuilder& Reset() {mNumColorAttachments = 0; return *this; }
		FPipelineRenderingBuilder& SetDepthAttachment(vk::Format format) { mDepthAttachmentFormat = format; return *this;}
		FPipelineRenderingBuilder& AddColorAttachment(vk::Format format)
		{
			mColorAttachmentFormats[mNumColorAttachments] = format;
			++mNumColorAttachments;

			return *this;
		}

	public:
		std::array<vk::Format, kMaxColorAttachments> mColorAttachmentFormats = {};
		u32 mNumColorAttachments = 0;

		vk::Format mDepthAttachmentFormat = vk::Format::eUndefined;
	};

	constexpr ConstString GetShaderEntryPointName(vk::ShaderStageFlagBits stage)
	{
		switch (stage)
		{
		case vk::ShaderStageFlagBits::eVertex:
			return "vsMain";
		case vk::ShaderStageFlagBits::eFragment:
			return "psMain";
		default:
			return "main";
		}
	}

	struct FShaderStateBuilder
	{
	public:
		FShaderStateBuilder& Reset() { mStagesCount = 0; return *this; }
		FShaderStateBuilder& AddStage(const std::string_view shaderName, vk::ShaderStageFlagBits stage, std::string_view entryPoint = "")
		{
			if (entryPoint.empty())
			{
				entryPoint = GetShaderEntryPointName(stage);
			}

			mStages[mStagesCount] = {std::string(shaderName), stage, std::string(entryPoint)};
			++mStagesCount;
			return *this;
		}
		FShaderStateBuilder& SetName(FName name) { mName = name; return *this; }

	public:
		std::array<FShaderStage, kMaxShaderStages> mStages;
		u32 mStagesCount = 0;

		FName mName;
	};

	struct FMultisampleStateBuilder
	{
		EMSAASamples mSamples : 4 = EMSAASamples::One;
		bool bSampleShading : 1 = false;
	};

	struct FPipelineBuilder
	{
		FPipelineBuilder& AddDescriptorSetLayout(THandle<FDescriptorSetLayout> handle)
		{
			mDescriptorSetLayouts[mNumActiveLayouts++] = handle;
			return *this;
		}

		template <PushConstant PushConstantType>
		FPipelineBuilder& SetPushConstantType() { mPushConstantSize = sizeof(PushConstantType); return *this; }

		FPipelineBuilder& SetPrimitiveTopology(vk::PrimitiveTopology primitiveTopology) { mTopology = primitiveTopology; return *this; }
		FPipelineBuilder& SetName(FName name) { mName = name; return *this; }

		[[nodiscard]] FName GetName() const { return mName; }

		FRasterizationBuilder mRasterizationBuilder;
		FDepthStencilBuilder mDepthStencilBuilder;
		FBlendStateBuilder mBlendStateBuilder;
		FShaderStateBuilder mShaderStateBuilder;
		FPipelineRenderingBuilder mPipelineRenderingBuilder;
		FMultisampleStateBuilder mMultisampleStateBuilder;

		std::array<THandle<FDescriptorSetLayout>, kMaxDescriptorSetLayouts> mDescriptorSetLayouts;
		u32 mNumActiveLayouts = 1; // The 0th set are always bindless resources

		u32 mPushConstantSize = 0;

		vk::PrimitiveTopology mTopology = vk::PrimitiveTopology::eTriangleList;

		FName mName = FName();
	};

	struct FCommandBufferBuilder
	{
		vk::CommandPool mVkCommandPool = nullptr;
		bool bPrimaryBuffer = true;
		FName mName = {};
	};

	struct FBLASBuilder
	{
		vk::Format mVertexFormat = vk::Format::eR32G32B32Sfloat;
		vk::IndexType mIndexType = vk::IndexType::eUint32;
		THandle<FBuffer> mVertexBuffer = {};
		THandle<FBuffer> mIndexBuffer = {};

		u32 mNumVertices = 0;

		vk::GeometryTypeKHR mGeometryType = vk::GeometryTypeKHR::eTriangles;
		vk::GeometryFlagsKHR mGeometryFlags = vk::GeometryFlagBitsKHR::eOpaque;

		FName mName = {};
	};

	struct FTLASBuilder
	{
		u32 mNumInstances = 0;
		FName mName = {};
	};

	struct FAccelerationStructureSizeInfo
	{
		FDeviceSize mAccelerationStructureSize;
		FDeviceSize mBuildScratchSize;
		FDeviceSize mUpdateScratchSize;
	};
}

#undef BUILDER_BODY
