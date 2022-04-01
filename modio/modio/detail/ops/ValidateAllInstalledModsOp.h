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
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/ValidateModInstallationOp.h"
#include <asio/yield.hpp>
#include <iterator>
#include <memory>

namespace Modio
{
	namespace Detail
	{
		/// @brief Class that loops through the current user's mod collection, and validates each one with
		/// ValidateModInstallationAsync
		class ValidateAllInstalledModsOp
		{
			asio::coroutine CoroutineState;
			std::map<Modio::ModID, std::shared_ptr<Modio::ModCollectionEntry>> UserModCollection;
			std::map<Modio::ModID, std::shared_ptr<Modio::ModCollectionEntry>>::iterator It;

		public:
			ValidateAllInstalledModsOp()
			{
				UserModCollection =
					Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions().Entries();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					// User has no subscriptions, no validation required
					if (UserModCollection.empty())
					{
						Self.complete({});
						return;
					}

					// Loop through current user's mods and validate those with ModState::Installed
					for (It = UserModCollection.begin(); It != UserModCollection.end(); It++)
					{
						if (It->second.get()->GetModState() == Modio::ModState::Installed)
						{
							yield Modio::Detail::ValidateModInstallationAsync(*It->second.get(), std::move(Self));

							// Set ModState to InstallationPending upon receiving an error
							if (ec)
							{
								Modio::Detail::Logger().Log(
									Modio::LogLevel::Error, Modio::LogCategory::ModManagement,
									"Setting ModState for mod with ID {} to InstallationPending",
									It->second.get()->GetID());
								It->second.get()->SetModState(Modio::ModState::InstallationPending);
							}
						}
					}
					// Self.complete WITHOUT ec so to not kill SDK initialization
					Self.complete({});
					return;
				}
			}
		};

		template<typename CompletionToken>
		void ValidateAllInstalledModsAsync(CompletionToken&& Callback)
		{
			return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
				ValidateAllInstalledModsOp(), Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>