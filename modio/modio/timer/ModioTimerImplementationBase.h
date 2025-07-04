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

#include "modio/detail/timer/ITimerImplementation.h"
#include <chrono>

namespace Modio
{
	namespace Detail
	{
		class TimerImplementationBase : public ITimerImplementation
		{
		protected:
			// std::chrono::steady_clock::time_point ExpiryTime;
			std::chrono::steady_clock::duration TimerDuration {};

		public:
			void ExpiresAfter(std::chrono::steady_clock::duration Duration) override
			{
				TimerDuration = Duration;
			}

			std::chrono::steady_clock::duration GetTimerDuration()
			{
				return TimerDuration;
			}

			/*void ExpireAt(std::chrono::steady_clock::time_point Time) override
			{
				ExpiryTime = Time;
			}*/

			void Cancel() override {}
		};
	} // namespace Detail
} // namespace Modio