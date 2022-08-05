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

	/// @brief Increments an implementation-defined counter
	/// @param Name Name of the counter to increment
	MODIO_WEAK(modio_profile_counter) void modio_profile_counter(const char* Name) MODIO_WEAK_STUB_IMPL;
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

#define MODIO_PROFILE_START() if (modio_profile_start != nullptr) modio_profile_start()
#define MODIO_PROFILE_END() modio_profile_stop()
#define MODIO_PROFILE_SAVE(FileName) modio_profile_save(#FileName)

#define MODIO_PROFILE_SCOPE(Name) \
	Modio::Detail::ScopedProfileEvent ScopedEvent_##Name = Modio::Detail::ScopedProfileEvent(#Name)

#define MODIO_PROFILE_PUSH(Name) modio_profile_push(#Name)
#define MODIO_PROFILE_POP() modio_profile_pop()

namespace Modio
{
	namespace Detail
	{
		class ScopedProfileEvent
		{
			const char* EventName;
			bool bMovedFrom = false;
			void* Data;

		public:
			~ScopedProfileEvent()
			{
				if (!bMovedFrom)
				{
					modio_profile_scope_end(EventName, Data);
				}
			}

			explicit ScopedProfileEvent(const char* EventName) : EventName(EventName)
			{
				modio_profile_scope_start(EventName, &Data);
			}

			ScopedProfileEvent(ScopedProfileEvent&& Other)
			{
				*this = std::move(Other);
			}

			ScopedProfileEvent& operator=(ScopedProfileEvent&& Other)
			{
				if (this != &Other)
				{
					EventName = Other.EventName;
					Other.bMovedFrom = true;
				}
				return *this;
			}
			ScopedProfileEvent(const ScopedProfileEvent& Other) = delete;
			ScopedProfileEvent& operator=(const ScopedProfileEvent& Other) = delete;
		};

	} // namespace Detail
} // namespace Modio
