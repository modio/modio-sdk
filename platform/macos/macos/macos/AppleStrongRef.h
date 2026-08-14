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
	#error "AppleStrongRef.h must only be included from Obj-C++ (.mm) translation units."
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
			// Owning handle for an Objective-C object: assigning a pointer retains
			// it, destroying the handle releases it. Under ARC the compiler already
			// emits that traffic, so the explicit calls compile away. Messages go
			// through (id) so classes that are only forward declared still work.
			template<typename ObjectType>
			class StrongRef
			{
			public:
				StrongRef() = default;

				// Retains Object. Use for the +0 results of factory methods such as
				// -dataTaskWithRequest:.
				explicit StrongRef(ObjectType* Object) : Held(Retained(Object)) {}

				// Takes over an existing +1 reference instead of retaining again. Pass
				// an -alloc/-init expression directly. NS_RELEASES_ARGUMENT states
				// that the reference transfers here, which is what ARC needs to
				// balance the call. Under manual reference counting the attribute is
				// documentation, so passing an object this handle does not own
				// over-releases it, and a CF object bridged in still owes its own
				// CFRelease.
				static StrongRef Adopt(ObjectType* NS_RELEASES_ARGUMENT Object)
				{
					StrongRef Ref;
					Ref.Held = Object;
					return Ref;
				}

				StrongRef(const StrongRef& Other) : Held(Retained(Other.Held)) {}

				// Under ARC the initializer retains and clearing Other releases, so
				// the reference transfers rather than duplicates.
				StrongRef(StrongRef&& Other) noexcept : Held(Other.Held) { Other.Held = nil; }

				StrongRef& operator=(const StrongRef& Other)
				{
					Reset(Other.Held);
					return *this;
				}

				StrongRef& operator=(StrongRef&& Other) noexcept
				{
					if (this != &Other)
					{
						Reset(Other.Held);
						Other.Reset();
					}
					return *this;
				}

				StrongRef& operator=(ObjectType* Object)
				{
					Reset(Object);
					return *this;
				}

				~StrongRef() { Released(Held); }

				// Retains before releasing, so self-assignment is safe. Under ARC
				// Previous is a strong reference, which keeps the old object alive.
				void Reset(ObjectType* Object = nil)
				{
					ObjectType* Previous = Held;
					Held = Retained(Object);
					Released(Previous);
				}

				ObjectType* Get() const { return Held; }

				explicit operator bool() const { return Held != nil; }

			private:
				static ObjectType* Retained(ObjectType* Object)
				{
#if MODIO_APPLE_HAS_ARC
					// Storing into the __strong member is the retain.
					return Object;
#else
					return [(id) Object retain];
#endif
				}

				static void Released(ObjectType* Object)
				{
#if MODIO_APPLE_HAS_ARC
					(void) Object;
#else
					[(id) Object release];
#endif
				}

				ObjectType* Held = nil;
			};

			// Wraps a +1 reference in a StrongRef without retaining it again, under
			// the same rules as StrongRef::Adopt.
			template<typename ObjectType>
			StrongRef<ObjectType> AdoptRef(ObjectType* NS_RELEASES_ARGUMENT Object)
			{
				return StrongRef<ObjectType>::Adopt(Object);
			}
		} // namespace Apple
	} // namespace Detail
} // namespace Modio
