#pragma once


namespace Turbo
{
   struct Engine;
   struct InputSystem;
   struct World;

	struct FActionEvent;
	struct FEventBase;

	namespace EditorFreeCameraUtils
	{
		void Init(Engine* engine);
		void RegisterEvents(InputSystem* inputSystem);
		void HandleEvent(FEventBase& Event, bool bViewportFocused);
		void Tick(World* world, fp32 deltaTime);

		void OnConstructMainViewPort(entt::registry& registry, const entt::entity& entity);
		void OnDestroyMainViewPort(entt::registry& registry, const entt::entity& entity);

		void HandleAction(FActionEvent& actionEvent, bool bViewportFocused = true);
		bool HandleEnableAction(FActionEvent& actionEvent, bool bViewportFocused = true);
		bool HandleMovementAction(FActionEvent& actionEvent);
		bool HandleRotationAction(FActionEvent& actionEvent);
		bool HandleChangeSpeedAction(FActionEvent& actionEvent);
	};
} // Turbo
