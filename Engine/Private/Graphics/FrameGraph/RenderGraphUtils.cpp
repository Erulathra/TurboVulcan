#include "Graphics/FrameGraph/RenderGraphUtils.h"

#include "Graphics/GPUDevice.h"

namespace Turbo
{
	void RenderGraphUtils::AddClearTexturePass(RenderGraph* renderGraph, FRGResourceHandle texture, glm::float4 color)
	{
		const FName passName(fmt::format("Clear {} to {}", renderGraph->GetTextureInfo(texture).mName, color));
		FRGPassInitializer pass = renderGraph->AddPass(passName, EPassType::Transfer);
		pass->WriteTexture(texture);

		pass->mExecutePass.BindLambda(
			[texture, color](GPUDevice* gpu, FCommandBuffer& cmd, FRenderResources& resources)
			{
				const THandle<FTexture> handle = resources.GetTexture(texture);
				cmd.ClearImage(handle, color, vk::ImageLayout::eTransferDstOptimal);
			}
		);
	}

	void RenderGraphUtils::AddBlitTexturePass(RenderGraph* renderGraph, FRGResourceHandle srcTexture, FRGResourceHandle dstTexture)
	{
		TURBO_CHECK(srcTexture != dstTexture)

		const FName passName(fmt::format(
			"Blit {} to {}",
			renderGraph->GetTextureInfo(srcTexture).mName,
			renderGraph->GetTextureInfo(dstTexture).mName)
		);

		FRGPassInitializer pass = renderGraph->AddPass(passName, EPassType::Transfer);
		pass->ReadTexture(srcTexture);
		pass->WriteTexture(dstTexture);

		pass->mExecutePass.BindLambda(
			[srcTexture, dstTexture](GPUDevice* gpu, FCommandBuffer& cmd, FRenderResources& resources)
			{
				const THandle<FTexture> srcHandle = resources.GetTexture(srcTexture);
				const THandle<FTexture> dstHandle = resources.GetTexture(dstTexture);

				const FTexture* colorTex= gpu->AccessTexture(srcHandle);
				const FTexture* presentTex= gpu->AccessTexture(dstHandle);

				const FRect2DInt srcRect = FRect2DInt::FromSize(colorTex->GetSize2D());
				const FRect2DInt dstRect = FRect2DInt::FromSize(presentTex->GetSize2D());

				cmd.BlitImage(srcHandle, srcRect, dstHandle, dstRect);
			}
		);
	}

	void RenderGraphUtils::AddFillBufferPass(
		RenderGraph* renderGraph,
		FRGResourceHandle srcBuffer,
		FDeviceSize offset,
		FDeviceSize size,
		u32 value
	)
	{
		const FName passName(fmt::format(
			"Fill {} (0x{:x}->0x{:x}) with 0x{:x}",
			renderGraph->GetBufferInfo(srcBuffer).mName,
			offset,
			size,
			value
		));

		FRGPassInitializer pass = renderGraph->AddPass(passName, EPassType::Transfer);
		pass->WriteBuffer(srcBuffer);

		pass->mExecutePass.BindLambda(
			[=](GPUDevice* gpu, FCommandBuffer& cmd, FRenderResources& resources)
			{
				const THandle<FBuffer> srcHandle = resources.GetBuffer(srcBuffer);
				cmd.FillBuffer(srcHandle, offset, size, value);
			}
		);
	}
} // Turbo
