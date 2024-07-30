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

#include "modio/detail/userdata/ModioUserDataContainer.h"

namespace Modio
{
	namespace Detail
	{
		inline void to_json(nlohmann::json& Json, const Modio::Detail::UserDataContainer& UserData)
		{
			Json = nlohmann::json::object(
				{UserData.UserSubscriptions,
				 {Modio::Detail::Constants::JSONKeys::DeferredUnsubscribes, UserData.DeferredUnsubscriptions}});
			if (UserData.AuthenticatedProfile)
			{
				Json[Modio::Detail::Constants::JSONKeys::OAuth] = UserData.AuthenticatedProfile->GetToken();
				Json[Modio::Detail::Constants::JSONKeys::UserProfile] = UserData.AuthenticatedProfile->GetUser();
				Json[Modio::Detail::Constants::JSONKeys::Avatar] = UserData.AuthenticatedProfile->GetAvatar();
			}
			if (UserData.UserModDirectoryOverride)
			{
				Json[Modio::Detail::Constants::JSONKeys::RootLocalStoragePath] =
					UserData.UserModDirectoryOverride.value().u8string();
			}
		}

		inline void from_json(const nlohmann::json& Json, Modio::Detail::UserDataContainer& UserData)
		{
			Modio::User AuthenticatedUser;
			bool ParseStatus =
				Modio::Detail::ParseSafe(Json, AuthenticatedUser, Modio::Detail::Constants::JSONKeys::UserProfile);
			ParseStatus &=
				Modio::Detail::ParseSafe(Json, AuthenticatedUser.Avatar, Modio::Detail::Constants::JSONKeys::Avatar);
			Modio::Detail::OAuthToken Token;
			ParseStatus &= Modio::Detail::ParseSafe(Json, Token, Modio::Detail::Constants::JSONKeys::OAuth);
			if (ParseStatus)
			{
				UserData.InitializeForUser(AuthenticatedUser, Token);
			}

			Modio::Detail::ParseSafe(Json, UserData.UserSubscriptions,
									 Modio::Detail::Constants::JSONKeys::UserSubscriptionList);
			Modio::Detail::ParseSafe(Json, UserData.DeferredUnsubscriptions,
									 Modio::Detail::Constants::JSONKeys::DeferredUnsubscribes);
			std::string TmpPath;
			Modio::Detail::ParseSafe(Json, TmpPath, Modio::Detail::Constants::JSONKeys::RootLocalStoragePath);
			if (!TmpPath.empty())
			{
				UserData.UserModDirectoryOverride = Modio::filesystem::path(TmpPath);
			}
		}

	} // namespace Detail
} // namespace Modio