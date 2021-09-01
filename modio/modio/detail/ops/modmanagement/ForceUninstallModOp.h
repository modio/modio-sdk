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

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class ForceUninstallModOp
		{
		public:
			ForceUninstallModOp(Modio::ModID ModToRemove) : ModToRemove(ModToRemove) {};
			
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState) {
					yield Modio::Detail::UninstallModAsync(ModToRemove, std::move(Self));
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
			Modio::ModID ModToRemove;
			asio::coroutine CoroutineState;
		};
	}
}
#include <asio/unyield.hpp>