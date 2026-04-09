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

#include "modio/core/ModioCoreTypes.h"

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Container for the error codes a response can store
		struct ResponseError
		{
			std::int32_t Code = -1;
			std::int32_t ErrorRef = -1;
			std::string Error {};
			Modio::Optional<std::vector<Modio::FieldError>> ExtendedErrorInformation {};
		};

	} // namespace Detail
} // namespace Modio