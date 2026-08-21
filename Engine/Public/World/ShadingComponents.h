#pragma once

#include "Graphics/ForwardLightningHelpers.h"

namespace Turbo
{
	struct FLightComponent
	{
		glm::float3 mColor = glm::float3(1.f);
		fp32 mIntensity = 1000.f;

		ELightType mType = ELightType::Point;
		fp32 mRange = 5.f;
		fp32 mInnerAngle = glm::radians(30.f);
		fp32 mOuterAngle = glm::radians(60.f);
	};
}
