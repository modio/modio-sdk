/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "ModioWeakSymbol.h"
#include "modio/core/ModioSplitCompilation.h"
#include <cstdint>
#include <utility>

#ifndef MODIO_WEAK_STUB_IMPL
	#define MODIO_WEAK_STUB_IMPL
#endif
extern "C"
{
	/// @brief Begins a profiling capture
	MODIO_WEAK(modio_profile_start) void modio_profile_start() MODIO_WEAK_STUB_IMPL;

	/// @brief Ends a profiling capture
	MODIO_WEAK(modio_profile_stop) void modio_profile_stop() MODIO_WEAK_STUB_IMPL;

	/// @brief Saves a profiling capture to the path indicated
	/// @param Name Name of the file to save at implementation-defined location
	MODIO_WEAK(modio_profile_save) void modio_profile_save(const char* Name) MODIO_WEAK_STUB_IMPL;

	/// @brief Increments an implementation-defined counter by one
	/// @param Name Name of the counter to increment
	/// @param Data Implementation-managed counter pointer
	MODIO_WEAK(modio_profile_counter_increment) void modio_profile_counter_increment(const char* Name, uint64_t* Data) MODIO_WEAK_STUB_IMPL;

	/// @brief Decrements an implementation-defined counter by one
	/// @param Name Name of the counter to decrement
	/// @param Data Implementation-managed counter pointer
	MODIO_WEAK(modio_profile_counter_decrement)
	void modio_profile_counter_decrement(const char* Name, uint64_t* Data) MODIO_WEAK_STUB_IMPL;

	/// @brief Sets an implementation-defined counter
	/// @param Name Name of the counter to decrement
	/// @param Data New value of counter
	MODIO_WEAK(modio_profile_counter_set)
	void modio_profile_counter_set(const char* Name, uint64_t Data) MODIO_WEAK_STUB_IMPL;

	/// @brief Begins a scoped profiling event
	/// @param Scope Name of the scope to start
	/// @param Data Implementation-managed context data pointer
	MODIO_WEAK(modio_profile_scope_start)
	void modio_profile_scope_start(const char* Scope, void** Data) MODIO_WEAK_STUB_IMPL;

	/// @brief Ends a scoped profiling event
	/// @param Scope Name of the scope to end
	/// @param Data Data pointer populated by <<modio_profile_scope_start>>
	MODIO_WEAK(modio_profile_scope_end)
	void modio_profile_scope_end(const char* Scope, void* Data) MODIO_WEAK_STUB_IMPL;

	MODIO_WEAK(modio_profile_push) void modio_profile_push(const char*) MODIO_WEAK_STUB_IMPL;
	MODIO_WEAK(modio_profile_pop) void modio_profile_pop() MODIO_WEAK_STUB_IMPL;
}

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief A profile event that falls within a certain scope
		class ScopedProfileEvent
		{
			const char* EventName;
			bool bMovedFrom = false;
			void* Data;

		public:
			/// @docinternal
			/// @brief Retrieve the event name
			/// @return Characters that form the event name
			const char* GetEventName()
			{
				return EventName;
			}

			/// @docinternal
			/// @brief Default destructor
			~ScopedProfileEvent()
			{
				if (!bMovedFrom)
				{
					modio_profile_scope_end(EventName, Data);
				}
			}

			/// @docinternal
			/// @brief Explicit constructor
			explicit ScopedProfileEvent(const char* EventName) : EventName(EventName)
			{
				modio_profile_scope_start(EventName, &Data);
			}

			/// @docinternal
			/// @brief ScopedProfileEvent move constructor
			ScopedProfileEvent(ScopedProfileEvent&& Other)
			{
				*this = std::move(Other);
			}

			/// @docinternal
			/// @brief ScopedProfileEvent assignment constructor
			MODIO_IMPL ScopedProfileEvent& operator=(ScopedProfileEvent&& Other);

			/// @docnone
			ScopedProfileEvent(const ScopedProfileEvent& Other) = delete;

			/// @docnone
			ScopedProfileEvent& operator=(const ScopedProfileEvent& Other) = delete;
		};

	} // namespace Detail
} // namespace Modio

#define MODIO_PROFILE_START()           \
	if (modio_profile_start != nullptr) \
	modio_profile_start()
#define MODIO_PROFILE_END() modio_profile_stop()
#define MODIO_PROFILE_SAVE(FileName) modio_profile_save(#FileName)

#define MODIO_PROFILE_COUNTER_INC(Name) modio_profile_counter_increment(#Name)
#define MODIO_PROFILE_COUNTER_DEC(Name) modio_profile_counter_decrement(#Name)
#define MODIO_PROFILE_COUNTER_SET(Name, Value) modio_profile_counter_set(#Name, Value)

#define MODIO_PROFILE_COUNTER_INC_NAMED(Name) modio_profile_counter_increment(Name)
#define MODIO_PROFILE_COUNTER_DEC_NAMED(Name) modio_profile_counter_decrement(Name)
#define MODIO_PROFILE_COUNTER_SET_NAMED(Name, Value) modio_profile_counter_set(Name, Value)


#define MODIO_PROFILE_SCOPE(Name) \
	Modio::Detail::ScopedProfileEvent ScopedEvent_##Name = Modio::Detail::ScopedProfileEvent(#Name)

#define MODIO_PROFILE_PUSH(Name) modio_profile_push(#Name)
#define MODIO_PROFILE_POP() modio_profile_pop()

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/ModioProfiling.ipp"
#endif