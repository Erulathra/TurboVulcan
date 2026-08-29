#pragma once
#include "RenderGraph.h"

namespace Turbo
{
	namespace RenderGraphUtils
	{
		void AddClearTexturePass(RenderGraph* renderGraph, FRGResourceHandle texture, glm::float4 color);
		void AddBlitTexturePass(RenderGraph* renderGraph, FRGResourceHandle srcTexture, FRGResourceHandle dstTexture);

		void AddFillBufferPass(
			RenderGraph* renderGraph,
			FRGResourceHandle srcBuffer,
			FDeviceSize offset,
			FDeviceSize size,
			u32 value
		);
	};
} // Turbo
