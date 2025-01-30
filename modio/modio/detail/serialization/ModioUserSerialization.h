/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioUser.h"

#include "modio/detail/serialization/ModioAvatarSerialization.h"

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::User& User)
	{
		Detail::ParseSafe(Json, User.UserId, "id");
		Detail::ParseSafe(Json, User.Username, "username");
		Detail::ParseSafe(Json, User.DateOnline, "date_online");
		Detail::ParseSafe(Json, User.ProfileUrl, "profile_url");
		nlohmann::json AvatarJsonData;
		if (Detail::GetSubobjectSafe(Json, "avatar", AvatarJsonData))
		{
			Modio::Detail::from_json(AvatarJsonData, User.Avatar);
		}

		Detail::ParseSafe(Json, User.DisplayNamePortal, "display_name_portal");
	}

	inline void to_json(nlohmann::json& Json, const Modio::User& User)
	{
		Json = nlohmann::json {{"id", User.UserId},
							   {"username", User.Username},
							   {"date_online", User.DateOnline},
							   {"profile_url", User.ProfileUrl},
							   {"avatar", User.Avatar},
							   {"display_name_portal", User.DisplayNamePortal}};
	}
} // namespace Modio
