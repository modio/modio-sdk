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

#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/ModioCompilerMacros.h"

MODIO_DISABLE_WARNING_PUSH
MODIO_DISABLE_WARNING_UNREACHABLE_CODE

namespace Modio
{
	namespace Detail
	{
		template<typename Exception>
		MODIO_NORETURN void MaybeThrowException(const Exception& e)
		{
#ifdef __cpp_exceptions
			throw e;
#else
			MODIO_UNREACHABLE(); abort();
#endif
		}

	} // namespace Detail
} // namespace Modio

MODIO_DISABLE_WARNING_POP
