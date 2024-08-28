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

namespace Modio
{
	namespace Detail
	{
		class ITimerImplementation
		{
		public:
			virtual ~ITimerImplementation() {}
			virtual void ExpiresAfter(std::chrono::steady_clock::duration Duration) = 0;
			//virtual void ExpireAt(std::chrono::steady_clock::time_point Time) = 0;
			virtual void Cancel() = 0;
		};
	} // namespace Detail
} // namespace Modio