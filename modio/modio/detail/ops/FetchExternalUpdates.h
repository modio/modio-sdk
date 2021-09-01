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
		class FetchExternalUpdatesOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {},
							Modio::Optional<Modio::ModInfoList> ServerSubscriptionList = {})
			{
				Modio::Detail::UserDataService& UserService =
					Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>();

				reenter(CoroutineState)
				{
					if (!Modio::Detail::SDKSessionData::GetAuthenticatedUser())
					{
						Self.complete(Modio::make_error_code(Modio::UserDataError::InvalidUser));
						return;
					}

					while (Modio::Detail::SDKSessionData::HasDeferredUnsubscriptions())
					{
						CurrentPendingUnsubscribe = Modio::Detail::SDKSessionData::GetDeferredUnsubscription();

						yield Modio::Detail::UnsubscribeFromModAsync(CurrentPendingUnsubscribe, std::move(Self));
						if (ec)
						{
							Self.complete(ec);
							return;
						}
						else
						{
							Modio::Detail::SDKSessionData::RemoveDeferredUnsubscription(CurrentPendingUnsubscribe);
						}
					}

					yield FetchUserSubscriptionsFromServerAsync(std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}

					{
						Modio::UserSubscriptionList ServerSubscriptionModIDs;
						std::map<Modio::ModID, Modio::ModInfo> ServerSubsModProfiles;
						if (!ServerSubscriptionList)
						{
							Self.complete(ec);
							return;
						}
						for (Modio::ModInfo& Profile : ServerSubscriptionList.value())
						{
							ServerSubscriptionModIDs.AddMod(Profile);
							ServerSubsModProfiles[Profile.ModId] = Profile;
							Modio::Detail::SDKSessionData::GetSystemModCollection().AddOrUpdateMod(
								Profile,
								Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().MakeModPath(
									Profile.ModId));
						}

						std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType> ModListDiff =
							UserSubscriptionList::CalculateChanges(
								Modio::Detail::SDKSessionData::GetUserSubscriptions(), ServerSubscriptionModIDs);
						bDirtyState = ModListDiff.size() > 0;
						for (auto ModChange : ModListDiff)
						{
							if (ModChange.second == Modio::UserSubscriptionList::ChangeType::Added)
							{
								Modio::Detail::SDKSessionData::GetUserSubscriptions().AddMod(
									ServerSubsModProfiles.at(ModChange.first));

								Modio::Optional<Modio::ModCollectionEntry&> ModEntry =
									Modio::Detail::SDKSessionData::GetSystemModCollection().GetByModID(ModChange.first);
								if (ModEntry.has_value())
								{
									ModEntry->AddLocalUserSubscription(
										Modio::Detail::SDKSessionData::GetAuthenticatedUser());
								}

								Modio::Detail::Logger().Log(LogLevel::Info, LogCategory::ModManagement,
															"External update adding {} to local user subscriptions",
															ModChange.first);
							}
							else if (ModChange.second == Modio::UserSubscriptionList::ChangeType::Removed)
							{
								Modio::Detail::SDKSessionData::GetUserSubscriptions().RemoveMod(ModChange.first);

								Modio::Optional<Modio::ModCollectionEntry&> ModEntry =
									Modio::Detail::SDKSessionData::GetSystemModCollection().GetByModID(ModChange.first);
								if (ModEntry.has_value())
								{
									ModEntry->RemoveLocalUserSubscription(
										Modio::Detail::SDKSessionData::GetAuthenticatedUser());
								}
								Modio::Detail::Logger().Log(LogLevel::Info, LogCategory::ModManagement,
															"External update removing {} from local user subscriptions",
															ModChange.first);
							}
						}
					}

					if (bDirtyState)
					{
						// Only save to storage if we actually have any changes to flush
						yield UserService.SaveUserDataToStorageAsync(std::move(Self));

						bDirtyState = false;
					}

					// Always save the mod collection to storage
					yield Modio::Detail::SaveModCollectionToStorageAsync(std::move(Self));

					Self.complete({});
					return;
				}
			}

		private:
			asio::coroutine CoroutineState;
			Modio::ModID CurrentPendingUnsubscribe;
			bool bDirtyState = false;
		};

		template<typename FetchDoneCallback>
		auto FetchExternalUpdatesAsync(FetchDoneCallback&& OnFetchComplete)
		{
			return asio::async_compose<FetchDoneCallback, void(Modio::ErrorCode)>(
				FetchExternalUpdatesOp(), OnFetchComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
