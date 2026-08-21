#pragma once

#include "Assets/AssetManagerHelpers.h"
#include "Core/DataStructures/Handle.h"
#include "Graphics/GraphicsCore.h"

namespace Turbo
{
	struct FAccelerationStructure;
	struct FBuffer;
	struct FMaterial;
	struct FBLAS;

	struct FBounds
	{
		glm::float3 mMin = glm::float3(std::numeric_limits<fp32>::max());
		fp32 mRadius = std::numeric_limits<fp32>::lowest();
		glm::float3 mMax = glm::float3(std::numeric_limits<fp32>::lowest());
		fp32 mRadiusSquared = std::numeric_limits<fp32>::lowest();
	};

	struct FMesh final
	{
		THandle<FBuffer> mIndexBuffer = {};
		THandle<FBuffer> mPositionBuffer = {};

		THandle<FBuffer> mNormalBuffer = {};
		THandle<FBuffer> mTangentBuffer = {};
		THandle<FBuffer> mUVBuffer = {};
		THandle<FBuffer> mColorBuffer = {};

		THandle<FBLAS> mBlas = {};

		FBounds mBounds;

		u32 mVertexCount = 0;

		THandle<FMesh> mHandle;
		FName mName;
		FAssetHash mAssetHash;

		[[nodiscard]] constexpr bool IsValid() const { return mHandle.IsValid(); }
		explicit constexpr operator bool() const { return IsValid(); }
	};

	struct FMeshData final
	{
		FDeviceAddress mIndexBuffer = kNullDeviceAddress;
		FDeviceAddress mPositionBuffer = kNullDeviceAddress;
		FDeviceAddress mNormalBuffer = kNullDeviceAddress;
		FDeviceAddress mTangentBuffer = kNullDeviceAddress;
		FDeviceAddress mUVBuffer = kNullDeviceAddress;

		u32 mVertexCount = 0;
		u32 mIndex = 0;
	};

} // Turbo
