#pragma once
#include "Core/Input/Input.h"

namespace Turbo
{
	class FRuntimeTestLayer : public ILayer
	{
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void OnEvent(FEventBase& event) override;
		virtual void BeginTick(fp64 deltaTime) override;
		virtual bool ShouldTick() override;

		virtual FName GetName() override;
	};
} // Turbo
