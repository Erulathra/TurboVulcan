#pragma once
#include "SceneGraph.h"
#include "Core/Math/FRotator.h"
#include "Graphics/ForwardLightningHelpers.h"

namespace Turbo
{
	enum class EProjectionType : u8
	{
		Perspective,
		Orthographic
	};

	struct FCamera final
	{
		fp32 mOrtoWidth = 10.f;
		fp32 mFov = glm::radians(60.f);
		fp32 mAspectRatio = 16.f / 9.f;

		fp32 mNearPlane = 0.1f;
		fp32 mFarPlane = 10000.f;

		EProjectionType mProjectionType = EProjectionType::Perspective;

		/** auto bindings */
		static void on_construct(entt::registry &registry, const entt::entity entity);
		static void on_update(entt::registry &registry, const entt::entity entity);
	};

	struct FCameraCache
	{
		glm::float4x4 mProjectionMatrix = {1.f};
		FFrustum mViewFrustum = {};
	};

	struct FFreeCamera
	{
		fp32 mMinMovementSpeed = 1.f;
		fp32 mMaxMovementSpeed = 1000.f;
		fp32 mMovementSpeedFactor = 1.1f;
		fp32 mMovementSpeed = 5.f;
		fp32 mRotationSensitivity = glm::radians(0.16f);
		FRotator mRotator = FRotator(0.f);
	};

	struct FProjectionDirty {};
	struct FMainViewport {};

	struct FViewData final
	{
		glm::float4x4 mProjectionMatrix = {1.f};
		glm::float4x4 mViewMatrix = {1.f};

		glm::float4x4 mWorldToProjection = {1.f};
		glm::float3 mCameraPosition = {};

		fp64 mTime = 0.f;
		fp64 mWorldTime = 0.f;
		fp64 mDeltaTime = 0.f;

		// In world space
		FFrustum mViewFrustum = {};

		fp32 mPreExposure = 1.f;
		fp32 mOneOverPreExposure = 1.f;
	};

	class FCameraUtils final
	{
	public:
		static void UpdateDirtyCameras(entt::registry& registry);
		static void UpdateFreeCameraPosition(entt::registry& registry, const glm::float3& movementInput, fp32 deltaTime);
		static void UpdateFreeCameraRotation(entt::registry& registry, const glm::float2& deltaRotation);
		static void UpdateFreeCameraSpeed(entt::registry& registry, const i32 deltaSpeed);
		static void UpdateCameraFrustum(entt::registry& registry);
		static FFrustum GetViewFrustum(const FCamera& camera, const FWorldTransform& transform);

	public:
		static void InitializeCamera(entt::registry& registry, entt::entity entity);
		static void InitializeFreeCamera(entt::registry& registry, entt::entity entity);
		static void SetMainViewport(entt::registry& registry, entt::entity entity);

		static entt::entity GetMainViewport(const entt::registry& registry);

	public:
		FCameraUtils() = delete;
	};

} // Turbo
