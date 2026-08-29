#include <ranges>

#include "Graphics/DestoryQueue.h"

namespace Turbo
{
	void FDestroyQueue::Flush(GPUDevice& GPUDevice)
	{
		for (TUniquePtr<IDestroyQueue>& destroyQueue : mDestroyQueues | std::views::values)
		{
			destroyQueue->Flush(GPUDevice);
		}

		mOnDestroy.Broadcast();
		mOnDestroy.RemoveAll();
	}
} // Turbo
