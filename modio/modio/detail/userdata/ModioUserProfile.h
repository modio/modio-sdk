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
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/core/entities/ModioToken.h"
#include "modio/core/entities/ModioUser.h"

#include <memory>

namespace Modio
{
	namespace Detail
	{
		/// @docpublic
		/// @brief User profile data (user data and OAuth token)
		struct ProfileData
		{
			/// @docpublic
			/// @brief Constructor for ProfileData
			ProfileData(Modio::User InUser, Modio::Detail::OAuthToken AccessToken) : User(InUser), Token(AccessToken) {}

			/// @docpublic
			/// @brief Get the user data
			const Modio::User& GetUser() const
			{
				return User;
			}

			/// @docpublic
			/// @brief Get the user's avatar
			Modio::Detail::Avatar GetAvatar() const
			{
				return User.Avatar;
			}

			/// @docinternal
			/// @brief Get the OAuth token
			const Modio::Detail::OAuthToken& GetToken() const
			{
				return Token;
			}

			/// @docinternal
			/// @brief Invalidate the OAuth token
			void InvalidateOAuthToken()
			{
				Token.SetInvalidState();
			}

		private:
			/// @brief The user data
			Modio::User User;

			/// @brief The OAuth token
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
