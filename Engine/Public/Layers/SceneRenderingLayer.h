#pragma once

#include "CommonTypeDefs.h"
#include "Core/DataStructures/Handle.h"
#include "Graphics/FrameGraph/RenderGraph.h"
#include "Graphics/FrameGraph/RenderGraphHelpers.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/Resources.h"
#include "World/Camera.h"
#include "World/World.h"

DECLARE_LOG_CATEGORY(LogSceneRendering, Display, Display)

namespace Turbo
{
   struct Engine;
	struct FBuffer;
	struct FMaterial;
	class FCommandBuffer;
	struct RenderGraph;
	struct CoreTimer;

	// Replace with growable buffer
	constexpr SizeType kNumAllocatedMaterialInstances = 512;

	struct SceneData final
	{
		u32 mNumLights = 0;
		u32 mSceneTLAS = 0;

		fp32 mAmbientLight = 0.03f;

		u32 _PADDING[2];
	};

	struct SceneView
	{
		// Those pointers are valid only during this frame
		FViewData* mViewData = nullptr;
		SceneData* mSceneData = nullptr;
		FLight* mLights = nullptr; // There is mNumLights in mSceneData;

		FRGResourceHandle mViewDataBufferHandle = {};
		FRGResourceHandle mSceneDataBufferHandle = {};
		FRGResourceHandle mLightsBufferHandle = {};

		// Ray-tracing
		THandle<FTLAS> mTLAS = {};
		FRGResourceHandle mTLASStorageBufferHandle = {};
	};

	struct DrawIndirectBucket
	{
		THandle<FMaterial> mMaterialHandle = {};
		u32 mCount = 0;
		FRGResourceHandle mIndirectCommandBuffer = {};
		FRGResourceHandle mDrawBuffer = {};
	};

	struct SceneRenderingLayer
	{
		THandle<FPipeline> mFrustumCullingPipeline;
		THandle<FPipeline> mToneMapperPipeline;

		/* Public interface */
		void Init(Engine* engine);
		void Shutdown(Engine* engine);

		void Render(Engine* engine);

		void RenderScene(Engine* engine, SceneView* SceneView);
		void RenderPostProcess(Engine* engine, SceneView* SceneView);

		static void UpdateViewData(Engine* engine, FViewData& viewData);
		static void CreateIndirectRenderBuffers(Engine* engine, SceneView* sceneView, std::vector<DrawIndirectBucket>& outBuckets);
		static void CreateSceneTLAS(Engine* engine, SceneView* sceneView);
	};
} // namespace Turbo
