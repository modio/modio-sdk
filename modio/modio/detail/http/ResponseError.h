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
#include "modio/detail/ModioJsonHelpers.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Modio
{
	namespace Detail
	{
		struct ResponseError
		{
			std::int32_t Code;
			std::int32_t ErrorRef;
			std::string Error;
			Modio::Optional<std::vector<FieldError>> ExtendedErrorInformation;
		};

		static void from_json(const nlohmann::json& Json, Modio::Detail::ResponseError& Error)
		{
			const nlohmann::json& Subobject = Json.at("error");
			if (Subobject.is_object())
			{
				Detail::ParseSafe(Subobject, Error.Code, "code");
				Detail::ParseSafe(Subobject, Error.ErrorRef, "error_ref");
				Detail::ParseSafe(Subobject, Error.Error, "message");

#if 0
				const nlohmann::json& ExtendedErrorMessage = Json.at("errors");
				if(ExtendedErrorMessage.is_object())
				{
					Error.ExtendedErrorInformation.emplace();
					std::vector<FieldError>& Var =
						Error.ExtendedErrorInformation.value();
					Detail::ParseSafe<std::vector<FieldError>>(Subobject, Var, "errors");
				}
#endif
			}
		}
	} // namespace Detail
} // namespace Modio