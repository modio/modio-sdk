/*
 *  Copyright (C) 2021-2023 mod.io Pty Ltd. <https://mod.io>
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
#include "modio/core/ModioServices.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/ops/SaveModCollectionToStorage.h"
#include "modio/detail/ops/UnsubscribeFromMod.h"
#include "modio/detail/ops/modmanagement/FetchUserSubscriptionsFromServer.h"
#include "modio/file/ModioFileService.h"
#include "modio/userdata/ModioUserDataService.h"
#include <algorithm>
#include <asio/coroutine.hpp>
#include <asio/yield.hpp>
#include <map>

namespace Modio
{
	namespace Detail
	{
		class PreviewExternalUpdatesOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {},
							Modio::Optional<Modio::ModInfoList> ServerSubscriptionList = {})
			{
				reenter(CoroutineState)
				{
					if (!Modio::Detail::SDKSessionData::GetAuthenticatedUser())
					{
						Self.complete(Modio::make_error_code(Modio::UserDataError::InvalidUser), {});
						return;
					}

					yield FetchUserSubscriptionsFromServerAsync(std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					if (ServerSubscriptionList.has_value() == false && ServerSubscriptionList.value().Size() <= 0)
					{
						Self.complete({}, {});
						return;
					}

					{
						Modio::UserSubscriptionList ServerSubscriptionModIDs;

						for (Modio::ModInfo& Profile : ServerSubscriptionList.value())
						{
							ServerSubscriptionModIDs.AddMod(Profile);
						}

						std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType> ModListDiff =
							UserSubscriptionList::CalculateChanges(
								Modio::Detail::SDKSessionData::GetUserSubscriptions(), ServerSubscriptionModIDs);

						Modio::ModCollection RemainingMods =
							Modio::Detail::SDKSessionData::GetSystemModCollection().FilterByUserSubscriptions(
								ServerSubscriptionModIDs);
						std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType> ModUpdates =
							UserSubscriptionList::CalculateUpdates(ServerSubscriptionList.value(), RemainingMods);

						// Insert the changes already calculated in ModListDiff. It gives priority to the
						// results from CalculateChanges, ex. "Added" over "Update"
						ModListDiff.insert(ModUpdates.begin(), ModUpdates.end());

						Self.complete({}, ModListDiff);
						return;
					}
				}
			}

		private:
			asio::coroutine CoroutineState;
		}; // PreviewExternalUpdatesOp

		template<typename PreviewDoneCallback>
		auto PreviewExternalUpdatesAsync(PreviewDoneCallback&& OnPreviewComplete)
		{
			return asio::async_compose<PreviewDoneCallback,
									   void(Modio::ErrorCode,
											std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType>)>(
				PreviewExternalUpdatesOp(), OnPreviewComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
