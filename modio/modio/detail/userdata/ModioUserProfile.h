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

#include "modio/core/ModioLogger.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/core/entities/ModioToken.h"

#include <memory>

namespace Modio
{
	namespace Detail
	{
		struct ProfileData
		{
			ProfileData(Modio::User InUser, Modio::Detail::OAuthToken AccessToken) : User(InUser), Token(AccessToken) {}

			const Modio::User& GetUser() const
			{
				return User;
			}

			const Modio::Detail::Avatar GetAvatar() const
			{
				return User.Avatar;
			}

			const Modio::Detail::OAuthToken& GetToken() const
			{
				return Token;
			}
			void InvalidateOAuthToken()
			{
				Token.SetInvalidState();
			}

		private:
			Modio::User User;
			Modio::Detail::OAuthToken Token;

			/// @docnone
			friend bool operator==(const Modio::Detail::ProfileData& A, const Modio::Detail::ProfileData& B)
			{
				return (A.User == B.User && A.Token == B.Token);
			}
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioUserProfile.ipp"
#endif
