#include "Core/RHI/Pipelines/ComputePipeline.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Core/RHI/VulkanDevice.h"

namespace Turbo
{
	void FComputePipeline::Init(const vk::ShaderModule& shaderModule)
	{
		CreatePipelineLayout();
		CreatePipeline(shaderModule);
	}

	void FComputePipeline::CreatePipelineLayout()
	{
		vk::PipelineLayoutCreateInfo layoutCreateInfo{};

		TURBO_CHECK(mDescriptorSetLayout);
		layoutCreateInfo.setSetLayouts({mDescriptorSetLayout});

		vk::PushConstantRange pushConstant{};
		pushConstant.setOffset(0);
		pushConstant.setSize(sizeof(float));
		pushConstant.setStageFlags(vk::ShaderStageFlagBits::eCompute);

		layoutCreateInfo.setPushConstantRanges(pushConstant);

		vk::Result result;
		std::tie(result, mPipelineLayout) = mDevice->Get().createPipelineLayout(layoutCreateInfo);
		CHECK_VULKAN_HPP(result);
	}

	void FComputePipeline::CreatePipeline(const vk::ShaderModule& shaderModule)
	{
		vk::PipelineShaderStageCreateInfo stageInfo{};
		stageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
		stageInfo.setModule(shaderModule);
		stageInfo.setPName("main"); // TODO: Hardcoded for now

		vk::ComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.setLayout(mPipelineLayout);
		computePipelineCreateInfo.setStage(stageInfo);

		vk::Result result;
		std::tie(result, mPipeline) = mDevice->Get().createComputePipeline(nullptr, computePipelineCreateInfo);
		CHECK_VULKAN_HPP(result);
	}

	void FComputePipeline::SetDescriptors(const vk::DescriptorSetLayout& layout, const vk::DescriptorSet& set)
	{
		mDescriptorSetLayout = layout;
		mDescriptorSet = set;
	}

	void FComputePipeline::Dispatch(const vk::CommandBuffer& cmd, glm::ivec3 groupCount)
	{
		cmd.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mPipelineLayout, 0, {mDescriptorSet}, {});

		const float time = static_cast<float>(FCoreTimer::TimeFromEngineStart());
		cmd.pushConstants<float>(mPipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, {time});
		cmd.dispatch(groupCount.x, groupCount.y, groupCount.z);
	}

	void FComputePipeline::Destroy()
	{
		mDevice->Get().destroyPipelineLayout(mPipelineLayout);
		mDevice->Get().destroyPipeline(mPipeline);
		mDevice->Get().destroyDescriptorSetLayout(mDescriptorSetLayout);
	}
} // Turbo
