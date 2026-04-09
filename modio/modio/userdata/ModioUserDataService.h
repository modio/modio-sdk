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

#include "modio/detail/ops/userdata/SaveUserDataToStorageOp.h"
#include "modio/detail/ops/userdata/InitializeUserDataOp.h"

namespace Modio
{
	namespace Detail
	{
		class UserDataService : public ModioAsio::detail::service_base<UserDataService>
		{
		public:
			explicit UserDataService(ModioAsio::io_context& IOService)
				: ModioAsio::detail::service_base<UserDataService>(IOService)
			{
				
			}
			UserDataService(UserDataService&&) = delete;

			void Shutdown()
			{
				
			}

			template<typename CompletionHandlerType>
			auto InitializeAsync(CompletionHandlerType&& Handler)
			{
				return ModioAsio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
					InitializeUserDataOp(), Handler, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionHandlerType>
			auto ClearUserDataAsync(CompletionHandlerType&& Handler, bool bShouldDisableModManagement = true)
			{
				ClearUserData(bShouldDisableModManagement);				
				return SaveUserDataToStorageAsync(std::forward<CompletionHandlerType>(Handler));			
			}

			template<typename CompletionHandlerType>
			auto SaveUserDataToStorageAsync(CompletionHandlerType&& Handler)
			{
				return ModioAsio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
					SaveUserDataToStorageOp(), Handler,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			MODIO_IMPL Modio::ErrorCode ApplyGlobalConfigOverrides(const std::map<std::string, std::string> MODIO_UNUSED_ARGUMENT(Overrides)) { return {};}

		private:
			void ClearUserData(bool bShouldDisableModManagement = true);
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioUserDataService.ipp"
#endif