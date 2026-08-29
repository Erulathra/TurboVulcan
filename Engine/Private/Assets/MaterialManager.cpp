#include "Assets/MaterialManager.h"
#include <vector>

#include "Core/DataStructures/Handle.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{

	void FMaterialManager::Init(GPUDevice* gpu)
	{
	   mGPU = gpu;
	}

	void FMaterialManager::Destroy(GPUDevice* gpu)
	{
		std::vector<THandle<FMaterial>> materialsToDestroy;
		materialsToDestroy.reserve(mMaterialPool.Size());

		for (const auto& [key, value] : mMaterialToMaterialInstanceMap)
		{
   		materialsToDestroy.push_back(key);
		}

		for (THandle<FMaterial> materialHandle : materialsToDestroy)
		{
			DestroyMaterial(materialHandle);
		}
	}

	FPipelineBuilder FMaterialManager::CreateOpaquePipeline(std::string_view shaderName)
	{
		FPipelineBuilder pipelineBuilder = {};
		pipelineBuilder
			.SetPushConstantType<FMaterial::PushConstants>()
			.SetName(FName(fmt::format("Material_{}", shaderName)));

		pipelineBuilder.mShaderStateBuilder
			.AddStage(shaderName, vk::ShaderStageFlagBits::eVertex)
			.AddStage(shaderName, vk::ShaderStageFlagBits::eFragment);

		pipelineBuilder.mBlendStateBuilder
			.AddNoBlendingState();

		pipelineBuilder.mDepthStencilBuilder
			.SetDepth(true, false, vk::CompareOp::eEqual);

		pipelineBuilder.mPipelineRenderingBuilder
			.AddColorAttachment(FGeometryBuffer::kColorFormat)
			.SetDepthAttachment(FGeometryBuffer::kDepthStencilFormat);

		return pipelineBuilder;
	}

	FPipelineBuilder FMaterialManager::CreateDepthPrepassPipeline(std::string_view shaderName)
	{
		FPipelineBuilder pipelineBuilder = {};
		pipelineBuilder
			.SetPushConstantType<FMaterial::PushConstants>()
			.SetName(FName(fmt::format("Material_{}_Depth", shaderName)));

		pipelineBuilder.mShaderStateBuilder
			.AddStage(shaderName, vk::ShaderStageFlagBits::eVertex);

		pipelineBuilder.mDepthStencilBuilder
			.SetDepth(true, true, vk::CompareOp::eGreaterOrEqual);

		pipelineBuilder.mPipelineRenderingBuilder
			.SetDepthAttachment(FGeometryBuffer::kDepthStencilFormat);

		return pipelineBuilder;
	}

	THandle<FMaterial> FMaterialManager::CreateMaterial(const FMaterialBuilder& builder)
	{
		TURBO_CHECK(builder.mGraphicsPipeline)

		const THandle<FPipeline> pipelineHandle = mGPU->CreatePipeline(*builder.mGraphicsPipeline);
		TURBO_CHECK(pipelineHandle);

		const THandle<FPipeline> depthPipelineHandle = mGPU->CreatePipeline(*builder.mDepthOnlyPipeline);
		TURBO_CHECK(depthPipelineHandle);

		const THandle<FMaterial> materialHandle = mMaterialPool.Acquire();
		FMaterial* material = mMaterialPool.Get(materialHandle);
		material->mGraphicsPipeline = pipelineHandle;
		material->mDepthOnlyPipeline = depthPipelineHandle;
		material->mDataBuffer = {};
		material->mMaterialDataSize = builder.mMaterialDataSize;
		material->mPerInstanceDataSize = builder.mPerInstanceDataSize;
		material->mMaxInstances = builder.mPerInstanceDataSize > 0 ? builder.mMaxInstances : 1;
		material->mName = builder.mName;

		const size_t targetBufferSize = builder.mMaterialDataSize + builder.mPerInstanceDataSize * builder.mMaxInstances;
		if (targetBufferSize > 0)
		{
			FBufferBuilder bufferBuilder = {};
			bufferBuilder
				.Init(EBufferFlags::CreateMapped | EBufferFlags::StorageBuffer, targetBufferSize)
				.SetName(FName(fmt::format("{}_Uniforms", builder.mGraphicsPipeline->GetName())));
			material->mDataBuffer = mGPU->CreateBuffer(bufferBuilder);
			TURBO_CHECK(material->mDataBuffer);
		}

		mMaterialToMaterialInstanceMap[materialHandle] = FMaterialInstanceArray();

		FAvailableIndexes availableIndexes = FAvailableIndexes();
		availableIndexes.resize(builder.mMaxInstances);

		for (size_t instanceId = 0; instanceId < builder.mMaxInstances; ++instanceId)
		{
			availableIndexes[instanceId] = builder.mMaxInstances - instanceId - 1;
		}

		mMaterialToAvailableIndexesMap[materialHandle] = std::move(availableIndexes);

		TURBO_CHECK_SLOW(mMaterialNameLookUp.find(builder.mName) == mMaterialNameLookUp.end())
		mMaterialNameLookUp[builder.mName] = materialHandle;

		return materialHandle;
	}

	THandle<FMaterial> FMaterialManager::GetMaterial(FName materialName)
	{
		if (auto foundIt = mMaterialNameLookUp.find(materialName);
			foundIt != mMaterialNameLookUp.end())
		{
			return foundIt->second;
		}

		return {};
	}

	THandle<FMaterial::Instance> FMaterialManager::CreateMaterialInstance(THandle<FMaterial> materialHandle)
	{
		FMaterial* material = AccessMaterial(materialHandle);
		TURBO_CHECK(material)

		FAvailableIndexes& availableIndexes = mMaterialToAvailableIndexesMap.at(materialHandle);
		TURBO_CHECK(!availableIndexes.empty())

		THandle<FMaterial::Instance> instanceHandle = mMaterialInstancePool.Acquire();
		FMaterial::Instance* instance = mMaterialInstancePool.Get(instanceHandle);

		instance->material = materialHandle;
		instance->mUniformBufferIndex = availableIndexes.back();
		availableIndexes.pop_back();

		return instanceHandle;
	}

	void FMaterialManager::UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, std::span<ByteType> data)
	{
		TRACE_ZONE_SCOPED();

		TRACE_GPU_SCOPED(mGPU, cmd, "Update Material Instance")

		const FMaterial::Instance* instance = mMaterialInstancePool.Get(instanceHandle);
		const FMaterial* material = mMaterialPool.Get(instance->material);

		TURBO_CHECK(data.size() == material->mPerInstanceDataSize)

		const FBuffer* instancesDataBuffer = mGPU->AccessBuffer(material->mDataBuffer);
		ByteType* targetInstanceAddress = instancesDataBuffer->mMappedAddress + CalculateInstanceByteOffset(*material, instance->mUniformBufferIndex);
		std::memcpy(targetInstanceAddress, data.data(), data.size());

		cmd.BufferBarrier(
			material->mDataBuffer,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eUniformRead,
			vk::PipelineStageFlagBits2::eVertexShader
			);
	}

	FDeviceAddress FMaterialManager::GetMaterialInstanceAddress(THandle<FMaterial::Instance> instanceHandle) const
	{
		if (const FMaterial::Instance* instance = mMaterialInstancePool.Get(instanceHandle))
		{
			if (const FMaterial* material = mMaterialPool.Get(instance->material);
				material->mDataBuffer.IsValid())
			{
				const FBuffer* uniformBuffer = mGPU->AccessBuffer(material->mDataBuffer);
				return uniformBuffer->mDeviceAddress + CalculateInstanceByteOffset(*material, instance->mUniformBufferIndex);
			}
		}

		return kNullDeviceAddress;
	}

	void FMaterialManager::UpdateMaterialData(FCommandBuffer& cmd, THandle<FMaterial> handle, std::span<ByteType> data)
	{
		TRACE_ZONE_SCOPED();

		TRACE_GPU_SCOPED(mGPU, cmd, "Update Material Data")

		const FMaterial* material = mMaterialPool.Get(handle);
		TURBO_CHECK(data.size() == material->mMaterialDataSize)

		const FBuffer* instancesDataBuffer = mGPU->AccessBuffer(material->mDataBuffer);
		std::memcpy(instancesDataBuffer->mMappedAddress, data.data(), data.size());

		cmd.BufferBarrier(
			material->mDataBuffer,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eUniformRead,
			vk::PipelineStageFlagBits2::eVertexShader
			);
	}

	FDeviceAddress FMaterialManager::GetMaterialDataAddress(THandle<FMaterial> handle) const
	{
		const FMaterial* material = mMaterialPool.Get(handle);
		if (material->mDataBuffer.IsValid())
		{
			const FBuffer* uniformBuffer = mGPU->AccessBuffer(material->mDataBuffer);
			return uniformBuffer->mDeviceAddress;
		}

		return kNullDeviceAddress;
	}

	SizeType FMaterialManager::CalculateInstanceByteOffset(const FMaterial& material, u32 instanceIndex)
	{
		return material.mMaterialDataSize + instanceIndex * material.mPerInstanceDataSize;
	}

	void FMaterialManager::DestroyMaterial(THandle<FMaterial> materialHandle)
	{
		TRACE_ZONE_SCOPED()


		const FMaterial* material = mMaterialPool.Get(materialHandle);
		TURBO_CHECK(material);
		mGPU->DestroyPipeline(material->mGraphicsPipeline);
		mGPU->DestroyPipeline(material->mDepthOnlyPipeline);

		if (material->mDataBuffer.IsValid())
		{
			mGPU->DestroyBuffer(material->mDataBuffer);
		}

		const FMaterialInstanceArray& instanceHandles = mMaterialToMaterialInstanceMap.at(materialHandle);
		for (const THandle materialInstanceHandle : instanceHandles)
		{
			mMaterialInstancePool.Release(materialInstanceHandle);
		}

		mMaterialToMaterialInstanceMap.erase(materialHandle);
		mMaterialToAvailableIndexesMap.erase(materialHandle);
	}

	void FMaterialManager::DestroyMaterialInstance(THandle<FMaterial::Instance> handle)
	{
		FMaterial::Instance* materialInstance = mMaterialInstancePool.Get(handle);
		TURBO_CHECK(materialInstance);

		FAvailableIndexes& availableIndexes = mMaterialToAvailableIndexesMap.at(materialInstance->material);
		availableIndexes.push_back(materialInstance->mUniformBufferIndex);

		FMaterialInstanceArray& instanceHandles = mMaterialToMaterialInstanceMap.at(materialInstance->material);
		instanceHandles.erase(handle);

		mMaterialInstancePool.Release(handle);
	}
}
