#pragma once

#include "MaterialManager.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{
	struct FMaterial;
	struct FMesh;

	namespace EngineMaterials
	{
		inline const FName kTriangleTest = FName("MeshTriangleTest");
		inline const FName kOpaqueBasePass = FName("OpaqueBasePass");

		void InitEngineMaterials();

		struct FBasePassMaterialData
		{
			u32 mBaseColorSampler = kInvalidBinding;
			u32 mMetalicRoughnessSampler = kInvalidBinding;
			u32 mNormalSampler = kInvalidBinding;
		};

		struct FBasePassInstanceData
		{
			u32 mBaseColorTexture = kInvalidBinding;
			u32 mMetalicRoughnessTexture = kInvalidBinding;
			u32 mNormalTexture = kInvalidBinding;

			fp32 mNormalScale = 1.f;
			glm::float4 mBaseColorFactor = glm::float4(1.f);
			fp32 mMetalicFactor = 0.f;
			fp32 mRoughnessFactor = 1.f;
		};
	}

	namespace EngineResources
	{
		void InitEngineSamplers();
		void InitEngineTextures();
		void LoadPlaceholders();
		void LoadPlaceholderMesh();

		void DestroyEngineResources();

		THandle<FSampler> GetDefaultLinearSampler();
		THandle<FSampler> GetDefaultNearestNeighbourSampler();

		THandle<FTexture> GetWhiteTexture();
		THandle<FTexture> GetBlackTexture();
		THandle<FTexture> GetPlaceholderTexture();
		THandle<FTexture> GetORMPlaceholderTexture();
		THandle<FTexture> GetFlatNormalMapTexture();

		THandle<FMesh> GetPlaceholderMesh();

		void GenerateCheckerboardTextureData(
			ByteType* outBytes,
			glm::uint2 size,
			std::span<const ByteType> onValue,
			std::span<const ByteType> offValue
		);
	}
}
