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
#include "ModioGeneratedVariables.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/entities/ModioAvatar.h"
#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief Object representing a mod.io user profile
	struct User
	{
		/// @brief Unique id for the user
		Modio::UserID UserId = Modio::UserID(0);

		/// @brief Username of the user
		std::string Username = "";

		/// @brief Unix timestamp the user was last online
		std::int64_t DateOnline = 0;

		/// @brief URL of the user's mod.io profile
		std::string ProfileUrl = "";

		/// @brief Cached information about the avatar
		Modio::Detail::Avatar Avatar;

		friend bool operator==(const Modio::User& A, const Modio::User& B)
		{
			return (A.UserId == B.UserId && A.Username == B.Username && A.DateOnline == B.DateOnline &&
					A.ProfileUrl == B.ProfileUrl && A.Avatar == B.Avatar);
		}
	};

	inline void from_json(const nlohmann::json& Json, Modio::User& User)
	{
		Detail::ParseSafe(Json, User.UserId, "id");
		Detail::ParseSafe(Json, User.Username, "username");
		Detail::ParseSafe(Json, User.DateOnline, "date_online");
		Detail::ParseSafe(Json, User.ProfileUrl, "profile_url");
		nlohmann::json AvatarJson;
		if (Detail::GetSubobjectSafe(Json, "avatar", AvatarJson))
		{
			Modio::Detail::from_json(AvatarJson, User.Avatar);
		}
	}
	inline void to_json(nlohmann::json& Json, const Modio::User& User)
	{
		Json = nlohmann::json {{"id", User.UserId},
							   {"username", User.Username},
							   {"date_online", User.DateOnline},
							   {"profile_url", User.ProfileUrl},
							   {"avatar", User.Avatar}};
	}

} // namespace Modio
