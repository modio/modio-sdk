/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/ModioProfiling.h"
#endif


Modio::Detail::ScopedProfileEvent& Modio::Detail::ScopedProfileEvent::operator=(ScopedProfileEvent&& Other)
{
	if (this != &Other)
	{
		EventName = Other.EventName;
		Other.bMovedFrom = true;
	}
	return *this;
}