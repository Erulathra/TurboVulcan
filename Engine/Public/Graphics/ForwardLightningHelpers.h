#pragma once

namespace Turbo
{
	enum class ELightType : u8
	{
		Point,
		Spot,
		Directional,

		MaxValue = Directional,
		Num
	};
	DEFINE_ENUM_OPERATORS(ELightType, u8)

	inline const char* ToString(ELightType light)
	{
		switch (light)
		{
		case ELightType::Point:
			return "Point";
		case ELightType::Spot:
			return "Spot";
		case ELightType::Directional:
			return "Directional";
		InvalidDefaultCase;
		}
	}

	// Update Light encoding code
	static_assert(static_cast<u8>(ELightType::MaxValue) < (1 << 2));

	struct FLight
	{
		glm::float3 mColor = glm::float3(1.f);
		fp32 mIntensity = 1000.f;
		glm::float3 mPosition = glm::float3(0.f);
		fp32 mRange = 5.f;

		glm::float3 mDirection = EFloat3::Forward;
		u32 mInnerOuterAngleAndType = 0;

		ByteType _PADDING[16];
	};

	namespace ForwardLightning
	{
		constexpr u32 EncodeLightAnglesAndType(fp32 InnerAngle, fp32 OuterAngle, ELightType lightType)
		{
			constexpr u32 MaxAngleValue = (1 << 15) - 1;

			const fp32 InnerAngleNorm = glm::clamp<fp32>(InnerAngle, 0.0f, M_PI) / M_PI;
			const u32 EncodedInnerAngle = static_cast<u32>(InnerAngleNorm * MaxAngleValue) & MaxAngleValue;
			const fp32 OuterAngleNorm = glm::clamp<fp32>(OuterAngle, 0.0f, M_PI) / M_PI;
			const u32 EncodedOuterAngle = static_cast<u32>(OuterAngleNorm * MaxAngleValue) & MaxAngleValue;

			return EncodedInnerAngle << 17 | EncodedOuterAngle << 2 | (static_cast<u32>(lightType) & 3);
		}

		constexpr u32 EncodeLightType(ELightType lightType)
		{
			constexpr u32 MaxType = static_cast<u32>(ELightType::MaxValue);
			return 0u | (static_cast<u32>(lightType) & MaxType);
		}
	}
}
