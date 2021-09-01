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

#include "modio/core/ModioStdTypes.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
		namespace Schema
		{
			struct AccessTokenObject
			{
				std::int32_t HttpResponseCode;
				std::string AccessToken;
				Modio::Timestamp DateExpires;
			};

			inline void from_json(const nlohmann::json& Json, AccessTokenObject& AccessToken)
			{
				Modio::Detail::ParseSafe(Json, AccessToken.HttpResponseCode, "code");
				Modio::Detail::ParseSafe(Json, AccessToken.AccessToken, "access_token");
				Modio::Detail::ParseSafe(Json, AccessToken.DateExpires, "date_expires");
			}
		} // namespace Schema
	} // namespace Detail
} // namespace Modio
