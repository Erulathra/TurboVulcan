#pragma once

namespace Turbo
{
   struct World;

	using FAssetHash = u32;

	// Replace with something more robust
	constexpr u32 kMaxMeshes = 1024;

	struct FMeshLoadSettings
	{
		u32 mMeshIndex = 0;
		u32 mSubMeshIndex = 0;
		World* mOwner = nullptr;
	};

	struct FTextureAsset
	{
		FAssetHash mAssetHash;
	};

	struct FTextureLoadingSettings
	{
		bool mbSRGB : 1 = true;
		World* mOwner = nullptr;
	};
}
