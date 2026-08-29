#pragma once

#include "Core/DataStructures/Handle.h"
#include "Graphics/FrameGraph/RenderGraph.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "Graphics/Resources.h"
#include "entt/locator/locator.hpp"

namespace Turbo::ToneMapperPostProcess
{
	struct FUniformBuffer
	{
   	fp32 mExposure;
      fp32 mOneOverPreExposure;

		fp32 mSaturation;

		glm::float3 mOffset;
		glm::float3 mSlope;
		glm::float3 mPower;
	};

	struct FPushConstants
	{
		u32 mSceneColor = kInvalidBinding;
		u32 mOutput = kInvalidBinding;
		glm::uint2 mTextureSize = {};

		FDeviceAddress mUniforms = kNullDeviceAddress;
	};

	inline THandle<FPipeline> CreatePipeline(THandle<FDescriptorSetLayout> graphBuilderSetLayout, GPUDevice* gpu)
	{
		FPipelineBuilder builder;
		builder
		   .AddDescriptorSetLayout(graphBuilderSetLayout)
			.SetPushConstantType<FPushConstants>()
			.SetName(FName("ToneMapperPostProcess"));

		builder.mShaderStateBuilder
			.AddStage("PostProcess/ToneMapperPostProcess", vk::ShaderStageFlagBits::eCompute);

		return gpu->CreatePipeline(builder);
	}
}
