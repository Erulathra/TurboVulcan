#pragma once

#include "Graphics/GPUDevice.h"

namespace Turbo
{
	struct FAsyncLoadingManager
	{
	public:
		void Init(FGPUDevice& gpu);
		void Shutdown(FGPUDevice& gpu);

		void Update();

	private:
		vk::CommandPool mVkCommandPool;
		TUniquePtr<FCommandBuffer> mCommandBuffer;
		vk::Semaphore mVkSemaphore;
		vk::Fence mVkFence;

		std::mutex mTextureRequestStackCS;
	};
}
