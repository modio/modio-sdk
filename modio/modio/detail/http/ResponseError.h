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
#include "modio/detail/JsonWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include <string>
#include <vector>

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Container for the error codes a response can store
		struct ResponseError
		{
			std::int32_t Code;
			std::int32_t ErrorRef;
			std::string Error;
			Modio::Optional<std::vector<Modio::FieldError>> ExtendedErrorInformation;

			MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::Detail::ResponseError& Error);
		};
	} // namespace Detail
} // namespace Modio