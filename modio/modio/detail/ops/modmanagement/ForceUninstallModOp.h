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
#include "modio/detail/ops/modmanagement/UninstallMod.h"
#include "modio/detail/ops/SaveModCollectionToStorage.h"
#include "modio/detail/AsioWrapper.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class ForceUninstallModOp
		{
		public:
			ForceUninstallModOp(Modio::ModID ModToRemove) : ModToRemove(ModToRemove) {}
			
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState) {
					yield Modio::Detail::UninstallModAsync(ModToRemove, std::move(Self), true);
					if (ec)
					{
						Self.complete(ec);
						return;
					}
					else
					{
						yield Modio::Detail::SaveModCollectionToStorageAsync(std::move(Self));

						Self.complete({});
						return;
					}
				}
			}

		private:
			Modio::ModID ModToRemove {};
			asio::coroutine CoroutineState {};
		};
#include <asio/unyield.hpp>

		template<typename UninstallCompleteCallback>
		void ForceUninstallModAsync(Modio::ModID ModToRemove, UninstallCompleteCallback&& OnUninstallComplete)
		{
			return asio::async_compose<UninstallCompleteCallback, void(Modio::ErrorCode)>(
				Modio::Detail::ForceUninstallModOp(ModToRemove), OnUninstallComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	}
}
