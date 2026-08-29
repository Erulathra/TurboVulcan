#pragma once

#include "Graphics/GPUDevice.h"

namespace Turbo
{
	struct FAsyncLoadingManager
	{
	public:
		void Init(GPUDevice& gpu);
		void Shutdown(GPUDevice& gpu);

		void Update();

	private:
		vk::CommandPool mVkCommandPool;
		TUniquePtr<FCommandBuffer> mCommandBuffer;
		vk::Semaphore mVkSemaphore;
		vk::Fence mVkFence;

		std::mutex mTextureRequestStackCS;
	};
}
