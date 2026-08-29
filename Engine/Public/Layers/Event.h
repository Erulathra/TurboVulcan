#pragma once

#include "Core/Utils/Templates.h"

/** Must be at begin of the class */
#define EVENT_BODY(EventType)																	\
public:																						\
static entt::id_type GetStaticTypeId() { return entt::type_id<EventType>().index(); }	\
EventType() : FEventBase(EventType::GetStaticTypeId()) {}								\

// TODO(SS): Refactor this to FatStruct

namespace Turbo
{
   struct Engine;

	enum class EEventReply
	{
		Unhandled = 0,
		Handled,
	};

	struct FEventBase
	{
		const entt::id_type mEventTypeId;
		EEventReply mEventReply = EEventReply::Unhandled;

		// This field would be filled by PushEvent
		Engine* mEngine = nullptr;

	public:
		void Handle() { mEventReply = EEventReply::Handled; }

	protected:
		explicit FEventBase(entt::id_type eventType)
			: mEventTypeId(eventType)
		{ }
	};

	namespace EventDispatcher
	{
		template <typename EventType, typename FunctionType, typename... Args>
		static void Dispatch(FEventBase& event, FunctionType callback, Args&&... args)
		{
			if (event.mEventTypeId == EventType::GetStaticTypeId() && event.mEventReply != EEventReply::Handled)
			{
				std::invoke(Forward<FunctionType>(callback), static_cast<EventType&>(event), Forward<Args>(args)...);
			}
		}

		template <typename FunctionType, typename... Args>
		static void Dispatch(FEventBase& event, FunctionType callback, Args&&... args)
		{
			if (event.mEventReply != EEventReply::Handled)
			{
				std::invoke(Forward<FunctionType>(callback), event, Forward<Args>(args)...);
			}
		}
	}; // namespace EventDispatcher

} // Turbo
