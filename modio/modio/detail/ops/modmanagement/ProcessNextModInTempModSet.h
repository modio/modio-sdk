/* 
 *  Copyright (C) 2024 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *   
 */

#pragma once

#include "modio/cache/ModioCacheService.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class ProcessNextModInTempModSet
		{
		public:
			ProcessNextModInTempModSet() 
			{
				ModId = Modio::ModID::InvalidModID();
			}
			
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState) 
				{  
					if (Modio::Detail::SDKSessionData::GetTemporaryModSet() != nullptr)
					{
						std::vector<Modio::ModID> tempModIdsToInstall = Modio::Detail::SDKSessionData::GetTemporaryModSet()->GetTempModIdsToInstall();

						if (tempModIdsToInstall.size() > 0)
							ModId = tempModIdsToInstall[0];
					}

					// ModId Not Found
					if (!ModId.IsValid())
					{
						Self.complete(ec);
						return;
					}

					{
						if (!Modio::Detail::SDKSessionData::IsModCacheInvalid(ModId))
						{
							ModInfoData = Services::GetGlobalService<CacheService>().FetchFromCache(ModId);
						}
					}

					if (!ModInfoData.has_value())
					{
						CachedResponse = Modio::Detail::SDKSessionData::IsModCacheInvalid(ModId)
											 ? Modio::Detail::CachedResponse::Disallow
											 : Modio::Detail::CachedResponse::Allow;

						yield Modio::Detail::PerformRequestAndGetResponseAsync(
							ResponseBodyBuffer,
							Modio::Detail::GetModRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
								.SetModID(ModId),
							CachedResponse, std::move(Self));

						if (ec)
						{
							Self.complete(ec);
							return;
						}

						{
							Modio::Detail::SDKSessionData::ClearModCacheInvalid(ModId);
							ModInfoData = TryMarshalResponse<Modio::ModInfo>(ResponseBodyBuffer);
							if (ModInfoData.has_value())
							{
								Services::GetGlobalService<CacheService>().AddToCache(ModInfoData.value());
							}
							else
							{
								Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
								return;
							}
						}
					}

					if (ModInfoData.has_value())
					{
						Modio::Detail::SDKSessionData::GetTemporaryModSet()->AddModInfoToTempModCollection(
							ModInfoData.value());
					}

					Self.complete({});
					return;
				}
			}

		private:
			Modio::ModID ModId;
			Modio::Detail::CachedResponse CachedResponse;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			asio::coroutine CoroutineState;
			Modio::Optional<Modio::ModInfo> ModInfoData;
		};

		template<typename ProcessNextCallback>
		auto ProcessNextModInTempModSetAsync(ProcessNextCallback&& OnProcessComplete)
		{
			return asio::async_compose<ProcessNextCallback, void(Modio::ErrorCode)>(
				ProcessNextModInTempModSet(), OnProcessComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	}
}
#include <asio/unyield.hpp>