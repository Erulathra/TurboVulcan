#pragma once

#include "Layers/Event.h"
#include "Math/Math.h"

namespace Turbo
{
	class Window;

	struct FCloseWindowEvent : FEventBase
	{
		EVENT_BODY(FCloseWindowEvent)
	};

	struct FResizeWindowEvent : FEventBase
	{
		EVENT_BODY(FResizeWindowEvent)

		glm::uint2 mNewWindowSize;
	};
}
