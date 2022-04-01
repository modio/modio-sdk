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
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/userdata/ModioUserProfile.h"
#include <vector>

namespace Modio
{
	namespace Detail
	{
		struct UserDataContainer
		{
			friend bool operator==(const Modio::Detail::UserDataContainer& A, const Modio::Detail::UserDataContainer& B)
			{
				return (A.UserSubscriptions == B.UserSubscriptions &&
						A.AuthenticatedProfile == B.AuthenticatedProfile &&
						A.DeferredUnsubscriptions == B.DeferredUnsubscriptions &&
						A.UserModDirectoryOverride == B.UserModDirectoryOverride);
			}

		public:
			MODIO_IMPL void ResetUserData();

			MODIO_IMPL void InitializeForUser(Modio::User AuthenticatedUser, Modio::Detail::OAuthToken AuthToken);

			// @todo: Making copy of user object
			MODIO_IMPL const Modio::Optional<Modio::User> GetAuthenticatedUser() const;

			// @todo: Making copy of avatar object
			MODIO_IMPL const Modio::Optional<Modio::Detail::Avatar> GetAuthenticatedUserAvatar() const;

			MODIO_IMPL const Modio::Optional<Modio::Detail::OAuthToken> GetAuthenticationToken() const;

			MODIO_IMPL const bool IsValid() const;

			Modio::UserSubscriptionList UserSubscriptions;
			std::vector<Modio::ModID> DeferredUnsubscriptions;
			Modio::Optional<Modio::Detail::ProfileData> AuthenticatedProfile;

			Modio::Optional<Modio::filesystem::path> UserModDirectoryOverride;

			friend void to_json(nlohmann::json& Json, const Modio::Detail::UserDataContainer& UserData)
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
			friend void from_json(const nlohmann::json& Json, Modio::Detail::UserDataContainer& UserData)
			{
				Modio::User AuthenticatedUser;
				bool ParseStatus =
					Modio::Detail::ParseSafe(Json, AuthenticatedUser, Modio::Detail::Constants::JSONKeys::UserProfile);
				ParseStatus &= Modio::Detail::ParseSafe(Json, AuthenticatedUser.Avatar,
														Modio::Detail::Constants::JSONKeys::Avatar);
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
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioUserDataContainer.ipp"
#endif
