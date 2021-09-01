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
			auto ClearUserDataAsync(CompletionHandlerType&& Handler)
			{
				ModCollection FilteredModCollection =
					Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();
				// This may require additional testing to ensure that we don't decrement the count further than we ought
				for (auto& Entry : FilteredModCollection.Entries())
				{
					Entry.second->RemoveLocalUserSubscription(Modio::Detail::SDKSessionData::GetAuthenticatedUser());
				}

				Modio::Detail::SDKSessionData::ClearUserData();
				return SaveUserDataToStorageAsync(std::forward<CompletionHandlerType>(Handler));			
			}

			template<typename CompletionHandlerType>
			auto SaveUserDataToStorageAsync(CompletionHandlerType&& Handler)
			{
				return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
					SaveUserDataToStorageOp(), Handler,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
			MODIO_IMPL Modio::ErrorCode ApplyGlobalConfigOverrides(const std::map<std::string, std::string> Overrides) { return {};}
		private:
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>