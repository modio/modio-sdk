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
#include "modio/core/entities/ModioUser.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/detail/ops/userdata/SaveUserDataToStorageOp.h"
#include "modio/detail/ops/userdata/InitializeUserDataOp.h"

#include "modio/detail/AsioWrapper.h"
#include <vector>

#include <asio/yield.hpp>

namespace Modio
{
	
	namespace Detail
	{
		class UserDataService : public asio::detail::service_base<UserDataService>
		{
		public:
			explicit UserDataService(asio::io_context& IOService)
				: asio::detail::service_base<UserDataService>(IOService)
			{
				
			}
			UserDataService(UserDataService&&) = delete;

			void Shutdown()
			{
				
			}

			template<typename CompletionHandlerType>
			auto InitializeAsync( CompletionHandlerType&& Handler)
			{
				return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
					InitializeUserDataOp(), Handler, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionHandlerType>
			auto ClearUserDataAsync(CompletionHandlerType&& Handler, bool bShouldDisableModManagement = true)
			{
				ModCollection FilteredModCollection =
					Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();
				// This may require additional testing to ensure that we don't decrement the count further than we ought
				for (auto& Entry : FilteredModCollection.Entries())
				{
					uint8_t Subscribers = Entry.second->RemoveLocalUserSubscription(Modio::Detail::SDKSessionData::GetAuthenticatedUser());
					
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
					Modio::ModCollection UserModCollection = Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();
					for (auto ModEntry : UserModCollection.Entries())
					{
						SDKSessionData::CancelModDownloadOrUpdate(ModEntry.first);
					}

					if (bShouldDisableModManagement)
					{
						Modio::DisableModManagement();	
					}
				}
				
				return SaveUserDataToStorageAsync(std::forward<CompletionHandlerType>(Handler));			
			}

			template<typename CompletionHandlerType>
			auto SaveUserDataToStorageAsync(CompletionHandlerType&& Handler)
			{
				return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
					SaveUserDataToStorageOp(), Handler,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
			MODIO_IMPL Modio::ErrorCode ApplyGlobalConfigOverrides(const std::map<std::string, std::string> MODIO_UNUSED_ARGUMENT(Overrides)) { return {};}
		private:
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
