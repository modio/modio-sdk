/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioSDKMultiplayerLibrary.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/ServiceInitializationOp.h"
#include "modio/detail/ops/Shutdown.h"
#include "modio/detail/ops/auth/AuthenticateUserExternal.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/detail/ModioConstants.h"
#include "modio/core/ModioDefaultRequestParameters.h"
#include <algorithm>
#include <asio/coroutine.hpp>
#include <asio/yield.hpp>
#include "modio/impl/SDKPostAsync.h"

namespace Modio
{
	namespace Detail
	{
		class InitializeModioServerOp
		{
		private:
			ModioAsio::coroutine CoroutineState;
			Modio::ServerInitializeOptions InitOptions;

		public:
			InitializeModioServerOp(Modio::ServerInitializeOptions InInitOptions) : InitOptions(InInitOptions) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				std::map<std::string, std::string> ModPathOverride;
				ModPathOverride.insert(std::pair<std::string, std::string>("RootLocalStoragePath", InitOptions.ModsDirectory));
				bool bFailedToOverridePath = false;

				// Init for non-user with the Auth Token, expire never (for now)
				Modio::User User;
				User.UserId = Modio::UserID(1);
				User.Username = "Modio Dedicated Server";
				Modio::Detail::OAuthToken AuthToken =
					Modio::Detail::OAuthToken(InitOptions.Token, INT_MAX);

				reenter(CoroutineState)
				{
					yield InitializeServiceAsync(InitOptions, std::move(Self));
					if (ec)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Core,
													"Failed to initialize Modio Server with error: {}", ec.message());
						Self.complete(ec);
						return;
					}

					ModioServer::Get().CachedServerInitializationOptions = InitOptions;

					// Override the mod path
					if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
							.ApplyGlobalConfigOverrides(ModPathOverride))
					{
						bFailedToOverridePath = true;
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Core,
													"Failed to Override the mod path for the server, error: {}",
													ec.message());
					}

					if (bFailedToOverridePath)
					{
						// Since we failed additional init steps, we shutdown.
						yield Modio::Detail::ShutdownAsync(Modio::Detail::Services::ResetGlobalContext(), std::move(Self));
						Self.complete(ec);
						return;
					}

					Modio::Detail::SDKSessionData::InitializeForUser(User, AuthToken);

					Self.complete({});
				}
			}
		};

		template<typename InitDoneCallback>
		auto InitializeModioServerAsync(Modio::ServerInitializeOptions InitOptions, InitDoneCallback&& OnInitComplete)
		{
			return ModioAsio::async_compose<InitDoneCallback, void(Modio::ErrorCode)>(
				InitializeModioServerOp(InitOptions), OnInitComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
