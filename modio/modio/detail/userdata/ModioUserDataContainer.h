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
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/userdata/ModioUserProfile.h"
#include <vector>

namespace Modio
{
	namespace Detail
	{
		struct UserDataContainer
		{
			/// @docnone
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
			
			MODIO_IMPL void UpdateTokenForExistingUser(Modio::Detail::OAuthToken AuthToken);

			// @todo: Making copy of user object
			MODIO_IMPL const Modio::Optional<Modio::User> GetAuthenticatedUser() const;

			// @todo: Making copy of avatar object
			MODIO_IMPL const Modio::Optional<Modio::Detail::Avatar> GetAuthenticatedUserAvatar() const;

			MODIO_IMPL const Modio::Optional<Modio::Detail::OAuthToken> GetAuthenticationToken() const;

			MODIO_IMPL void InvalidateOAuthToken();

			MODIO_IMPL bool IsValid() const;

			Modio::UserSubscriptionList UserSubscriptions {};
			std::vector<Modio::ModID> DeferredUnsubscriptions {};
			Modio::Optional<Modio::Detail::ProfileData> AuthenticatedProfile {};

			Modio::Optional<Modio::filesystem::path> UserModDirectoryOverride {};

			/// @docnone
			MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::Detail::UserDataContainer& UserData);

			/// @docnone
			MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::Detail::UserDataContainer& UserData);
			
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioUserDataContainer.ipp"
#endif
