#pragma once

#include "Core/DataStructures/GenPool.h"
#include "Core/DataStructures/Handle.h"
#include "Graphics/ResourceBuilders.h"

namespace Turbo
{
	struct FBuffer;
	struct FPipeline;
	class FCommandBuffer;

	struct FMaterial final
	{
		using FUniformBufferIndex = u32;
		static constexpr FUniformBufferIndex kInvalidUniformBufferIndex = std::numeric_limits<FUniformBufferIndex>::max();

		struct Instance final
		{
			THandle<FMaterial> material = {};
			THandle<FMaterial::Instance> mHandle = {};
			u32 mUniformBufferIndex = kInvalidUniformBufferIndex;

			[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
			explicit constexpr operator bool() const { return IsValid(); }
		};

		struct IndirectDrawData final
		{
			glm::float4x4 mModelToProj;
			glm::float4x4 mModelToView;
			glm::float4x4 mModelToWorld;
			glm::float3x3 mNormalModelToWorld;

			FDeviceAddress mMaterialData = kNullDeviceAddress;
			FDeviceAddress mMaterialInstance = kNullDeviceAddress;
			FDeviceAddress mMeshData = kNullDeviceAddress;
		};

		struct PushConstants final
		{
			FDeviceAddress mViewData = kNullDeviceAddress;
			FDeviceAddress mSceneData = kNullDeviceAddress;
			FDeviceAddress mLightData = kNullDeviceAddress;

			FDeviceAddress mDrawData = kNullDeviceAddress;
		};

		THandle<FPipeline> mGraphicsPipeline = {};
		THandle<FPipeline> mDepthOnlyPipeline = {};
		THandle<FBuffer> mDataBuffer = {};
		u32 mPerInstanceDataSize = 0;
		u32 mMaterialDataSize = 0;
		u32 mMaxInstances = 0;

		THandle<FMaterial> mHandle;
		FName mName = {};

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const { return IsValid(); }
	};

	struct FMaterialBuilder
	{
		FPipelineBuilder* mGraphicsPipeline = nullptr;
		FPipelineBuilder* mDepthOnlyPipeline = nullptr;
		size_t mMaxInstances = 1;

		size_t mMaterialDataSize = 0;
		size_t mPerInstanceDataSize = 0;

		FName mName = {};
	};

	class FMaterialManager final
	{
		DELETE_COPY(FMaterialManager);

	public:
		FMaterialManager() = default;
		void Init(FGPUDevice& gpuDevice);
		void Destroy(FGPUDevice& gpuDevice);

	public:
		static FPipelineBuilder CreateOpaquePipeline(std::string_view shaderName);
		static FPipelineBuilder CreateDepthPrepassPipeline(std::string_view shaderName);

	public:
		THandle<FMaterial> CreateMaterial(const FMaterialBuilder& builder);

		[[nodiscard]] THandle<FMaterial> GetMaterial(FName materialName);

		THandle<FMaterial::Instance> CreateMaterialInstance(THandle<FMaterial> materialHandle);

		template<typename PerInstanceData>
		void UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, PerInstanceData* data);
		void UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, std::span<ByteType> data);
		[[nodiscard]] FDeviceAddress GetMaterialInstanceAddress(const FGPUDevice& gpu, THandle<FMaterial::Instance> instanceHandle) const;

		template<typename MaterialData>
		void UpdateMaterialData(FCommandBuffer& cmd, THandle<FMaterial> handle, MaterialData* data);
		auto UpdateMaterialData(FCommandBuffer& cmd, THandle<FMaterial> handle, std::span<ByteType> data) -> void;
		[[nodiscard]] FDeviceAddress GetMaterialDataAddress(const FGPUDevice& gpu, THandle<FMaterial> handle) const;

	public:
		[[nodiscard]] static size_t CalculateInstanceByteOffset(const FMaterial& material, u32 instanceIndex);

	public:
		[[nodiscard]] FMaterial* AccessMaterial(THandle<FMaterial> handle) { return mMaterialPool.Get(handle); }
		[[nodiscard]] const FMaterial* AccessMaterial(THandle<FMaterial> handle) const { return mMaterialPool.Get(handle); }
		[[nodiscard]] FMaterial::Instance* AccessInstance(THandle<FMaterial::Instance> handle) { return  mMaterialInstancePool.Get(handle); }
		[[nodiscard]] const FMaterial::Instance* AccessInstance(THandle<FMaterial::Instance> handle) const { return  mMaterialInstancePool.Get(handle); }

	public:
		void DestroyMaterial(THandle<FMaterial> materialHandle);
		void DestroyMaterialInstance(THandle<FMaterial::Instance> handle);

	private:
		TGenPool<FMaterial, 128> mMaterialPool;
		TGenPool<FMaterial::Instance, 2048> mMaterialInstancePool;

		using FMaterialInstanceArray = entt::dense_set<THandle<FMaterial::Instance>>;
		using FMaterialToMaterialInstanceMap = entt::dense_map<THandle<FMaterial>, FMaterialInstanceArray>;
		FMaterialToMaterialInstanceMap mMaterialToMaterialInstanceMap;

		using FAvailableIndexes = std::vector<u32>;
		using FMaterialToAvailableIndexes = entt::dense_map<THandle<FMaterial>, FAvailableIndexes>;
		FMaterialToAvailableIndexes mMaterialToAvailableIndexesMap;

		entt::dense_map<FName, THandle<FMaterial>> mMaterialNameLookUp;
	};

	template <typename PerInstanceData>
	void FMaterialManager::UpdateMaterialInstance(FCommandBuffer& cmd, THandle<FMaterial::Instance> instanceHandle, PerInstanceData* data)
	{
		UpdateMaterialInstance(cmd, instanceHandle, std::span<ByteType>(reinterpret_cast<ByteType*>(data), sizeof(PerInstanceData)));
	}

	template <typename MaterialData>
	void FMaterialManager::UpdateMaterialData(FCommandBuffer& cmd, THandle<FMaterial> handle, MaterialData* data)
	{
		UpdateMaterialData(cmd, handle, std::span<ByteType>(reinterpret_cast<ByteType*>(data), sizeof(MaterialData)));
	}
} // Turbo
