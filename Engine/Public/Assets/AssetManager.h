#pragma once

#include "Assets/StaticMesh.h"
#include "Assets/AssetManagerHelpers.h"
#include "Core/DataStructures/GenPool.h"
#include "Core/DataStructures/ManualPoolGrowable.h"
#include "Graphics/Resources.h"

DECLARE_LOG_CATEGORY(LogAssetManager, Display, Display)
DECLARE_LOG_CATEGORY(LogMeshLoading, Display, Display)
DECLARE_LOG_CATEGORY(LogTextureLoading, Display, Display)

namespace fastgltf
{
	class Asset;
}

namespace Turbo
{
	class FBuffer;
	class FTexture;
	struct FMesh;
	class GPUDevice;

	class FAssetManager
	{
		DELETE_COPY(FAssetManager);

	public:
		FAssetManager() = default;

	public:
		void Init(GPUDevice* gpu);
		void Destroy(GPUDevice* gpu) const;

		/** Mesh interface */
	public:
		[[nodiscard]] THandle<FMesh> LoadMesh(FName assetPath, const FMeshLoadSettings& meshLoadSettings = FMeshLoadSettings());
		[[nodiscard]] THandle<FMesh> LoadMeshGLTF(FName assetPath, const FMeshLoadSettings& meshLoadSettings, fastgltf::Asset& loadedAsset);

		void UnloadMesh(THandle<FMesh> meshHandle);

		[[nodiscard]] FMesh* AccessMesh(THandle<FMesh> handle) { return mMeshPool.Get(handle); }
		[[nodiscard]] const FMesh* AccessMesh(THandle<FMesh> handle) const { return mMeshPool.Get(handle); }

		[[nodiscard]] FDeviceAddress GetMeshPointersAddress(THandle<FMesh> handle) const;
		[[nodiscard]] FDeviceAddress GetBoundsAddress() const;

		/** Mesh interface end */

		/** Texture interface */
	public:
		[[nodiscard]] THandle<FTexture> LoadTexture(FName path, const FTextureLoadingSettings& loadingSettings = {});
		void UnloadTexture(THandle<FTexture> handle);

	private:
		THandle<FTexture> LoadDDS(FName path, const FTextureLoadingSettings& loadingSettings);

		/** Texture interface end */

	private:
		template<typename AssetType>
		THandle<AssetType> FindCachedAsset(u32 hash)
		{
			if (auto foundIt = mAssetCache.find(hash);
				foundIt != mAssetCache.end())
			{
				TURBO_LOG(LogAssetManager, Display, "Asset manager cache hit!")
				return THandle<AssetType>(foundIt->second);
			}

			return {};
		}

	private:
		GPUDevice* mGPU;

		TGenPool<FMesh, 2048> mMeshPool;
		THandle<FBuffer> mMeshPointersPool;
		THandle<FBuffer> mBoundsPool;

		TManualPoolGrowable<FTextureAsset> mTexturePool;

		entt::dense_map<u32, FHandle> mAssetCache;
	};
} // Turbo
