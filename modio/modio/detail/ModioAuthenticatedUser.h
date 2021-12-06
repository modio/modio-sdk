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

#include "modio/core/entities/ModioUser.h"
#include "modio/detail/entities/ModioAvatar.h"

namespace Modio
{
	namespace Detail
	{
		struct AuthenticatedUser
		{
			/** Public user data */
			Modio::User User;
			/** Cached information about the avatar */
			Modio::Detail::Avatar Avatar;

			friend bool operator==(const Modio::Detail::AuthenticatedUser& A, const Modio::Detail::AuthenticatedUser& B)
			{
				return (A.User == B.User && A.Avatar == B.Avatar);
			}
		};

		static void from_json(const nlohmann::json& Json, AuthenticatedUser& OutUser)
		{
			Modio::from_json(Json, OutUser.User);
			nlohmann::json AvatarJson;
			if (GetSubobjectSafe(Json, "avatar", AvatarJson))
			{
				Modio::Detail::from_json(AvatarJson, OutUser.Avatar);
			}
		}

		static void to_json(nlohmann::json& Json, const AuthenticatedUser& User)
		{
			Modio::to_json(Json, User.User);
			nlohmann::json AvatarJson;
			Modio::Detail::to_json(AvatarJson, User.Avatar);
			Json["avatar"] = std::move(AvatarJson);
		}
	} // namespace Detail
} // namespace Modio