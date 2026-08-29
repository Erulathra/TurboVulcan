#pragma once

namespace Turbo
{
	struct RenderGraph;
	class Window;
	class GPUDevice;
	class FCommandBuffer;

#if WITH_DEBUG_RENDERING_FEATURES

	class FScopedLabelRegion final
	{
	public:
		FScopedLabelRegion(FCommandBuffer& commandBuffer, FName label, glm::float4 color = glm::float4(1.f));
		~FScopedLabelRegion();

		DELETE_COPY(FScopedLabelRegion)
	private:
		FCommandBuffer* mCommandBuffer = nullptr;
#if WITH_PROFILER
		tracy::VkCtxScope* mGPUZone = nullptr;
		tracy::ScopedZone* mCPUZone = nullptr;
#endif // WITH_PROFILER
	};

#define DEBUG_LABEL_REGION(commandBuffer, label) FScopedLabelRegion __label_region__(commandBuffer, label);
#define DEBUG_LABEL_REGION_COLOR(commandBuffer, label, color) FScopedLabelRegion __label_region__(commandBuffer, label, color);
#else // WITH_DEBUG_RENDERING_FEATURES
#define DEBUG_LABEL_REGION(commandBuffer, label) {}
#define DEBUG_LABEL_REGION_COLOR(commandBuffer, label, color) {}
#endif // else WITH_DEBUG_RENDERING_FEATURES

	class IFrameDebuggerAPI
	{
	public:
		static void Emplace();
		virtual ~IFrameDebuggerAPI() = default;

	public:
		virtual bool Init() = 0;
		virtual void Shutdown() = 0;

		virtual bool CanCapture() { return false; }

		virtual void BeginCapture(GPUDevice* gpu, Window* window) = 0;
		virtual void EndCapture(GPUDevice* gpu, Window* window) = 0;

		virtual void CaptureFrame() = 0;
	};

	class FNullFrameDebuggerAPI final : public IFrameDebuggerAPI
	{
	public:
		virtual bool Init() override { return true; }
		virtual void Shutdown() override {}
		virtual void BeginCapture(GPUDevice* gpu, Window* window) override {}
		virtual void EndCapture(GPUDevice* gpu, Window* window) override {}
		virtual void CaptureFrame() override {}
	};

	class FScopedRenderCapture final
	{
	public:
		FScopedRenderCapture() = delete;
		FScopedRenderCapture(bool bCapture, RenderGraph& graphBuilder);
		~FScopedRenderCapture();

		DELETE_COPY(FScopedRenderCapture)

	private:
		RenderGraph* mGraphBuilder = nullptr;
	};
} // Turbo
