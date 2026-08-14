/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#ifndef __OBJC__
	#error "AppleScopedSemaphore.h must only be included from Obj-C++ (.mm) translation units."
#endif

#import <Foundation/Foundation.h>

#ifndef MODIO_APPLE_HAS_ARC
	#if defined(__has_feature) && __has_feature(objc_arc)
		#define MODIO_APPLE_HAS_ARC 1
	#else
		#define MODIO_APPLE_HAS_ARC 0
	#endif
#endif

namespace Modio
{
	namespace Detail
	{
		namespace Apple
		{
			// dispatch_release is unavailable under ARC, which manages dispatch
			// objects itself.
			class ScopedSemaphore
			{
			public:
				explicit ScopedSemaphore(long InitialValue)
					: Semaphore(dispatch_semaphore_create(InitialValue))
				{}

				ScopedSemaphore(const ScopedSemaphore&) = delete;
				ScopedSemaphore& operator=(const ScopedSemaphore&) = delete;

				~ScopedSemaphore()
				{
#if !MODIO_APPLE_HAS_ARC
					if (Semaphore != nullptr)
					{
						dispatch_release(Semaphore);
					}
#endif
				}

				void Signal() const { dispatch_semaphore_signal(Semaphore); }

				void WaitForever() const { dispatch_semaphore_wait(Semaphore, DISPATCH_TIME_FOREVER); }

			private:
				dispatch_semaphore_t Semaphore;
			};
		} // namespace Apple
	} // namespace Detail
} // namespace Modio
