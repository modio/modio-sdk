/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

// Implementation header - do not include directly
#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/ModioSDK.h"
#else
	#pragma once
#endif

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/ModioStringHelpers.h"
#include "modio/detail/ops/AuthenticateUserByEmailOp.h"
#include "modio/detail/ops/RequestEmailAuthCodeOp.h"
#include "modio/detail/ops/auth/AuthenticateUserByDiscord.h"
#include "modio/detail/ops/auth/AuthenticateUserByEpic.h"
#include "modio/detail/ops/auth/AuthenticateUserByGog.h"
#include "modio/detail/ops/auth/AuthenticateUserByItch.h"
#include "modio/detail/ops/auth/AuthenticateUserByOculus.h"
#include "modio/detail/ops/auth/AuthenticateUserByOpenID.h"
#include "modio/detail/ops/auth/AuthenticateUserByPSN.h"
#include "modio/detail/ops/auth/AuthenticateUserBySteam.h"
#include "modio/detail/ops/auth/AuthenticateUserBySwitchID.h"
#include "modio/detail/ops/auth/AuthenticateUserByXBoxLive.h"
#include "modio/detail/ops/auth/ModioGetTermsOfUseOp.h"
#include "modio/detail/ops/user/GetUserMediaOp.h"
#include "modio/detail/ops/userdata/RefreshUserDataOp.h"
#include "modio/detail/ops/userdata/VerifyUserAuthenticationOp.h"
#include "modio/impl/SDKPreconditionChecks.h"
#include "modio/userdata/ModioUserDataService.h"

namespace Modio
{
	void RequestEmailAuthCodeAsync(Modio::EmailAddress EmailAddress, std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([EmailAddress, Callback = std::move(Callback)]() mutable {
			asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
				Modio::Detail::RequestEmailAuthCodeOp(EmailAddress), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		});
	}

	// Deprecated version to be removed
	void GetTermsOfUseAsync(Modio::AuthenticationProvider Provider, Modio::Language Locale,
							std::function<void(Modio::ErrorCode, Modio::Optional<Modio::Terms> Terms)> Callback)
	{
		GetTermsOfUseAsync(Locale, Callback);
	}

	void GetTermsOfUseAsync(Modio::Language Locale,
							std::function<void(Modio::ErrorCode, Modio::Optional<Modio::Terms> Terms)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Locale, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback))
			{
				asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::Terms>)>,
									void(Modio::ErrorCode, Modio::Optional<Modio::Terms>)>(
					Modio::Detail::GetTermsOfUseOp(Locale), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

	void AuthenticateUserExternalAsync(Modio::AuthenticationParams User, Modio::AuthenticationProvider Provider,
									   std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([User, Provider, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) == false &&
				Modio::Detail::RequireNotRateLimited(Callback) == false)
			{
				return;
			}

			// Check if the User's AuthToken needs URL encoding
			if (User.bURLEncodeAuthToken == true)
			{
				User.AuthToken = Modio::Detail::String::URLEncode(User.AuthToken);
			}

			switch (Provider)
			{
				case AuthenticationProvider::GoG:
					Modio::Detail::AuthenticateUserByGoGAsync(User, Callback);
					break;
				case AuthenticationProvider::Itch:
					Modio::Detail::AuthenticateUserByItchAsync(User, Callback);
					break;
				case AuthenticationProvider::Steam:
					Modio::Detail::AuthenticateUserBySteamAsync(User, Callback);
					break;
				case AuthenticationProvider::XboxLive:
					Modio::Detail::AuthenticateUserByXBoxLiveAsync(User, Callback);
					break;
				case AuthenticationProvider::Switch:
					Modio::Detail::AuthenticateUserBySwitchIDAsync(User, Callback);
					break;
				case AuthenticationProvider::Discord:
					Modio::Detail::AuthenticateUserByDiscordAsync(User, Callback);
					break;
				case AuthenticationProvider::PSN:
					Modio::Detail::AuthenticateUserByPSNAsync(User, Callback);
					break;
				case AuthenticationProvider::Epic:
					Modio::Detail::AuthenticateUserByEpicAsync(User, Callback);
					break;
				case AuthenticationProvider::Oculus:
					// Oculus requires extended parameters to be sent along, so we validate the parameters
					// with our precondition check here.
					if (Modio::Detail::RequireValidOculusExtendedParameters(User, Callback) == false)
					{
						return;
					}
					Modio::Detail::AuthenticateUserByOculusAsync(User, Callback);
					break;
				case AuthenticationProvider::OpenID:
					Modio::Detail::AuthenticateUserByOpenIDAsync(User, Callback);
					break;
			}
		});
		// Return immediatelly if the SDK is not initialized and
		// The API rate limit is reached
	}

	void AuthenticateUserEmailAsync(Modio::EmailAuthCode AuthenticationCode,
									std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([AuthenticationCode, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback))
			{
				return asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
					Modio::Detail::AuthenticateUserByEmailOp(AuthenticationCode), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

	void ClearUserDataAsync(std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback))
			{
				return Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>().ClearUserDataAsync(
					Callback);
			}
		});
	}

	void VerifyUserAuthenticationAsync(std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback))
			{
				Modio::Detail::VerifyUserAuthenticationAsync(Callback);
			}
		});
	}

	void RefreshUserDataAsync(std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback))
			{
				Modio::Detail::RefreshUserDataAsync(Callback);
			}
		});
	}

	Modio::Optional<Modio::User> QueryUserProfile()
	{
		auto Lock = Modio::Detail::SDKSessionData::GetReadLock();
		if (Modio::Detail::SDKSessionData::IsInitialized())
		{
			return Modio::Detail::SDKSessionData::GetAuthenticatedUser();
		}
		return {};
	}

	void GetUserMediaAsync(Modio::AvatarSize AvatarSize,
						   std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([AvatarSize, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback))
			{
				return asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)>,
										   void(Modio::ErrorCode, Modio::Optional<std::string>)>(
					Modio::Detail::GetUserMediaOp(Modio::Detail::SDKSessionData::CurrentGameID(),
												  Modio::Detail::SDKSessionData::CurrentAPIKey(), AvatarSize),
					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

} // namespace Modio
