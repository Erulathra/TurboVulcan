#include "Graphics/ResourceBuilders.h"

constexpr std::array kSupportedDescriptors = {
	vk::DescriptorType::eSampler,
	vk::DescriptorType::eSampledImage,
	vk::DescriptorType::eStorageImage,
	vk::DescriptorType::eUniformBuffer,
	vk::DescriptorType::eStorageBuffer,
};

Turbo::FBufferBuilder Turbo::FBufferBuilder::CreateStagingBuffer(const void* data, u32 size)
{
	static const FName kStagingBufferName("Staging");

	FBufferBuilder result = {};
	result
		.Init(EBufferFlags::CreateMapped | EBufferFlags::TransferSrc, size)
		.SetData(data)
		.SetName(kStagingBufferName);

	return result;
}

Turbo::FBufferBuilder Turbo::FBufferBuilder::CreateStagingBuffer(u32 size)
{
	return CreateStagingBuffer(nullptr, size);
}

Turbo::FBufferBuilder Turbo::FBufferBuilder::CreateStagingBuffer(std::span<ByteType> data)
{
	return CreateStagingBuffer(data.data(), data.size());
}

Turbo::FBufferBuilder Turbo::FBufferBuilder::CreateScratchBuffer(u32 size)
{
	static const FName kStagingBufferName("Scratch");

	FBufferBuilder result = {};
	result
		.Init(EBufferFlags::AccelerationStructureStorage | EBufferFlags::AccelerationStructureInput | EBufferFlags::TransferSrc | EBufferFlags::StorageBuffer, size)
		.SetName(kStagingBufferName);

	return result;
}

Turbo::FDescriptorPoolBuilder::FDescriptorPoolBuilder()
{
	Reset();
}

Turbo::FDescriptorPoolBuilder& Turbo::FDescriptorPoolBuilder::Reset()
{
	mMaxSets = 0;

	for (vk::DescriptorType type : kSupportedDescriptors)
	{
		SetPoolRatio(type, 1.f);
	}

	return *this;
}

Turbo::FDescriptorPoolBuilder& Turbo::FDescriptorPoolBuilder::SetPoolRatio(vk::DescriptorType type, fp32 ratio)
{
	mPoolSizes[type] = ratio;
	return *this;
}
