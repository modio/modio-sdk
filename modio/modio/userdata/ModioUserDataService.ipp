/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/userdata/ModioUserDataService.h"
#endif

#include "modio/cache/ModioCacheService.h"

namespace Modio
{
	void DisableModManagement();
	Modio::Optional<Modio::ModProgressInfo> QueryCurrentModUpdate();

	namespace Detail
	{
		void UserDataService::ClearUserData(bool bShouldDisableModManagement)
		{
			ModCollection FilteredModCollection =
				Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();
			// This may require additional testing to ensure that we don't decrement the count further than we ought
			for (auto& Entry : FilteredModCollection.Entries())
			{
				uint8_t Subscribers =
					Entry.second->RemoveLocalUserSubscription(Modio::Detail::SDKSessionData::GetAuthenticatedUser());

				if (Subscribers == 0)
				{
					SDKSessionData::ClearModCacheInvalid(Entry.first);
				}
			}

			Modio::Detail::SDKSessionData::ClearUserData();
			Modio::Detail::Services::GetGlobalService<Modio::Detail::CacheService>().ClearCache();

			// This block makes sure to cancel any mod in progress and disable mod management
			{
				Modio::Optional<Modio::ModProgressInfo> CurrentMod = Modio::QueryCurrentModUpdate();

				if (CurrentMod.has_value() == true)
				{
					SDKSessionData::CancelModDownloadOrUpdate(CurrentMod.value().ID);
				}

				// Make sure to cancel any operation by the user subscription
				Modio::ModCollection UserModCollection =
					Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();
				for (auto ModEntry : UserModCollection.Entries())
				{
					SDKSessionData::CancelModDownloadOrUpdate(ModEntry.first);
				}

				if (bShouldDisableModManagement)
				{
					Modio::DisableModManagement();
				}
			}
		}
	} // namespace Detail
} // namespace Modio
