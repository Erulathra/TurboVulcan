#include "Graphics/Resources.h"
#include "Graphics/GPUDevice.h"

namespace Turbo {
	void FBufferDestroyer::Destroy(GPUDevice& GPUDevice)
	{
		GPUDevice.DestroyBufferImmediate(*this);
	}

	void FSamplerDestroyer::Destroy(GPUDevice& GPUDevice)
	{
		GPUDevice.DestroySamplerImmediate(*this);
	}

	void FTextureDestroyer::Destroy(GPUDevice& GPUDevice)
	{
		GPUDevice.DestroyTextureImmediate(*this);
	}

	void FShaderStateDestroyer::Destroy(GPUDevice& GPUDevice)
	{
		GPUDevice.DestroyShaderStateImmediate(*this);
	}

	void FDescriptorSetLayoutDestroyer::Destroy(GPUDevice& GPUDevice)
	{
		GPUDevice.DestroyDescriptorSetLayoutImmediate(*this);
	}

	void FDescriptorPoolDestroyer::Destroy(GPUDevice& GPUDevice)
	{
		GPUDevice.DestroyDescriptorPoolImmediate(*this);
	}

	void FPipelineDestroyer::Destroy(GPUDevice& GPUDevice)
	{
		GPUDevice.DestroyPipelineImmediate(*this);
	}

	void FAccelerationStructureDestroyer::Destroy(GPUDevice& GPUDevice)
	{
		GPUDevice.DestroyAccelerationStructureImmediate(*this);
	}
}
