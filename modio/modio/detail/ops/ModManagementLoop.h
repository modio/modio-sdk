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
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/FetchExternalUpdates.h"
#include "modio/detail/ops/modmanagement/ProcessNextModInUserCollection.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/timer/ModioTimer.h"
#include <asio/yield.hpp>
#include <memory>
namespace Modio
{
	namespace Detail
	{
		/// @brief Internal operation that processes the next mod entry in the current collection that requires some
		/// kind of action (update, installation, uninstallation)
		class ModManagementLoop
		{
			Modio::Detail::Timer IdleTimer;
			asio::coroutine CoroutineState;
			std::uint8_t ExternalUpdateCounter = 0;

		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if (ec == Modio::GenericError::OperationCanceled)
				{
					Self.complete(ec);
					return;
				}

				reenter(CoroutineState)
				{
					while (Modio::Detail::SDKSessionData::IsModManagementEnabled())
					{
						if (ExternalUpdateCounter == 0)
						{
							// yield Modio::Detail::FetchExternalUpdatesAsync(std::move(Self));
						}
						// When we get an authentication error, we should invalidate the current token to require reauth
						if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
						{
							Modio::Detail::SDKSessionData::InvalidateOAuthToken();
						}
						if (ec)
						{
							Self.complete(ec);
							return;
						}

						// This operation gets the user subscriptions, filters the system mod list based on those, then
						// processes the next mod in that filtered list that requires some kind of management operation
						// (installation, update, etc).
						// It flags the mod with any error state that it encounters, so we don't need to handle that
						// here, just sleep and try to process the next mod
						yield Modio::Detail::ProcessNextModInUserCollectionAsync(std::move(Self));

						
						// Sleep for one second
						IdleTimer.ExpiresAfter(std::chrono::seconds(1));
						yield IdleTimer.WaitAsync(std::move(Self));

						ExternalUpdateCounter++;
						ExternalUpdateCounter = ExternalUpdateCounter % 15;
					}
					Self.complete({});
				}
			}
		};

		template<typename ManagementLoopEndCallback>
		auto BeginModManagementLoopAsync(ManagementLoopEndCallback&& OnLoopEnded)
		{
			return asio::async_compose<ManagementLoopEndCallback, void(Modio::ErrorCode)>(
				ModManagementLoop(), OnLoopEnded, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>