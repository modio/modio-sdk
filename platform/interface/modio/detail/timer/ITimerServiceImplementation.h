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
		class ITimerServiceImplementation
		{
		public:
			virtual ~ITimerServiceImplementation() {}
			virtual void Shutdown() = 0;
			//virtual void Cancel(IOObjectImplementationType) = 0;
			//template<typename CompletionToken>
			//auto WaitAsync(IOObjectImplementationType PlatformIOObjectInstance, CompletionToken&& Token);

		};
	} // namespace Detail
} // namespace Modio
