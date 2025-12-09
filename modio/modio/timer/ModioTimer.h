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

#include "modio/core/ModioLogEnum.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/timer/ModioTimerService.h"
#include <chrono>

namespace Modio
{
	namespace Detail
	{
		/// @brief Generic threadsafe logging object. Can either construct and retain an instance, or log via a
		/// temporary
		class Timer : public ModioAsio::basic_io_object<Modio::Detail::TimerService>
		{
		protected:
		public:
			/// @brief Explicit constructor for a Timer that posts messages via an explicit io_context
			/// @param Context the io_context to use
			explicit Timer(ModioAsio::io_context& Context) : ModioAsio::basic_io_object<Modio::Detail::TimerService>(Context) {}

			/// @brief Explicit convenience constructor for a Timer that posts messages via the global SDK io_context
			explicit Timer()
				: ModioAsio::basic_io_object<Modio::Detail::TimerService>(Modio::Detail::Services::GetGlobalContext())
			{}

			Timer(Timer&& Other) : ModioAsio::basic_io_object<Modio::Detail::TimerService>(std::move(Other)) {}
			Timer& operator=(Timer&& Other) = default; 	

			void ExpiresAfter(std::chrono::steady_clock::duration Duration)
			{
				get_implementation()->ExpiresAfter(Duration);
			}

			/*void ExpireAt(std::chrono::steady_clock::time_point Time)
			{
				get_implementation().ExpireAt(Time);
			}*/

			void Cancel()
			{
				get_service().Cancel(get_implementation());
			}

			template<typename CompletionTokenType>
			auto WaitAsync(CompletionTokenType&& Token)
			{
				return get_service().WaitAsync(get_implementation(),
											   std::forward<CompletionTokenType>(Token));
			}
		};
	} // namespace Detail
} // namespace Modio