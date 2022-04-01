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
	#include "modio/cache/ModioCacheService.h"
#endif

#include "modio/core/ModioLogger.h"
#include "modio/core/ModioLogEnum.h"
#include "modio/core/ModioBuffer.h"

namespace Modio
{
	namespace Detail
	{
		CacheService::CacheService(asio::io_context& IOService)
			: asio::detail::service_base<CacheService>(IOService)
		{
			CacheInstance = std::make_shared<Cache>();
		}

		void CacheService::Shutdown()
		{
			// Cancel all timers
			for(auto& CacheEntry : CacheInstance->CacheEntries)
			{
				CacheEntry.second.Timer->cancel();	
			}
			ClearCache();
		}

		void CacheService::construct(implementation_type& Implementation) {}

		void CacheService::destroy(implementation_type& Implementation) {}

		void CacheService::SetCacheExpireTime(std::chrono::steady_clock::duration ExpireTime)
		{
			CacheExpiryTime = ExpireTime;
		}

		void CacheService::AddToCache(std::string ResourceURL, Modio::Detail::DynamicBuffer ResponseData)
		{
			auto Hasher = std::hash<std::string>();
			std::uint32_t URLHash = (std::uint32_t) Hasher(ResourceURL);

			// Don't add the instance again as if the instance is already in there, then we will destroy the old timer and it will expire causing unnecessary removals
			if(CacheInstance->CacheEntries.find(URLHash) == std::end(CacheInstance->CacheEntries))
			{
				// @todo-optimize This will fragment the heap quite much, rewrite using another container so we don't need to allocate so many objects on the heap
				std::unique_ptr<asio::steady_timer> CacheExpiryTimer = std::make_unique<asio::steady_timer>(get_io_context());
				CacheExpiryTimer->expires_after(CacheExpiryTime);

				Modio::Detail::Logger().Log(LogLevel::Info, LogCategory::Http, "Adding hash {} to cache", URLHash);

				auto DeleteCacheEntry = [WeakCacheReference = std::weak_ptr<Cache>(CacheInstance), URLHash](std::error_code) {
					std::shared_ptr<Cache> CacheReference = WeakCacheReference.lock();
					Modio::Detail::Logger().Log(LogLevel::Info, LogCategory::Http, "Removing hash {} from cache", URLHash);
					if (CacheReference)
					{
						CacheReference->CacheEntries.erase(URLHash);
					}
				};

				CacheExpiryTimer->async_wait(DeleteCacheEntry);

				CacheEntry Entry = { std::move(CacheExpiryTimer), std::move(ResponseData) };
				CacheInstance->CacheEntries.emplace(URLHash, std::move(Entry) );
			}
		}

		void CacheService::AddToCache(Modio::ModInfo ModInfoDetails)
		{
			Modio::Detail::Logger().Log(LogLevel::Info, LogCategory::Http, "Adding ModID {} to cache", ModInfoDetails.ModId);
			// The ModInfoCache would clean only when the mod.io SDK session ends. For that reason there is no
			// timer for this case. Another way to remove this is by calling "ClearCache"
			CacheInstance->ModInfoCache.emplace(ModInfoDetails.ModId, ModInfoDetails);
		}

		void CacheService::AddToCache(Modio::GameID GameIDDetail, Modio::ModInfoList ModInfoDetails)
		{
			Modio::Detail::Logger().Log(LogLevel::Info, LogCategory::Http, "Adding ModIDList to cache with GameID: {}",
										GameIDDetail);
			std::vector<Modio::ModID> ModIDVec;
			
			// Instead of keeping the whole list (with possible data replications), it adds single elements to the cache.
			for_each(ModInfoDetails.begin(), ModInfoDetails.end(),
					 [this, &ModIDVec](Modio::ModInfo ModInfoData) 
				{ 
					this->AddToCache(ModInfoData); 
					ModIDVec.push_back(ModInfoData.ModId);
				});

			CacheInstance->ModInfoListCache.emplace(GameIDDetail, ModIDVec);
		}

		Modio::Optional<Modio::Detail::DynamicBuffer> CacheService::FetchFromCache(std::string ResourceURL) const
		{
			auto Hasher = std::hash<std::string>();
			std::uint32_t URLHash = (std::uint32_t) Hasher(ResourceURL);

			auto CacheEntryIterator = CacheInstance->CacheEntries.find(URLHash);
			if (CacheEntryIterator != CacheInstance->CacheEntries.end())
			{
				return (CacheEntryIterator)->second.Data;
			}
			else
			{
				return {};
			}
		}

		Modio::Optional<Modio::ModInfo> CacheService::FetchFromCache(Modio::ModID ModIDDetail) const
		{
			auto CacheEntryIterator = CacheInstance->ModInfoCache.find(ModIDDetail);
			if (CacheEntryIterator != CacheInstance->ModInfoCache.end())
			{
				return CacheEntryIterator->second;
			}
			else
			{
				return {};
			}
		}

		Modio::Optional<Modio::ModInfoList> CacheService::FetchFromCache(Modio::GameID GameIDDetails) const 
		{
			auto CacheEntryIterator = CacheInstance->ModInfoListCache.find(GameIDDetails);
			if (CacheEntryIterator == CacheInstance->ModInfoListCache.end())
			{
				return {};
			}

			Modio::ModInfoList ModElems = {};

			for (Modio::ModID ModIDDetail : CacheEntryIterator->second) 
			{
				Modio::Optional<Modio::ModInfo> OpModInfo = FetchFromCache(ModIDDetail);
				if (OpModInfo.has_value() == false) 
				{
					// In case at least one of the ModID requested is not found in the ModInfoCache, then it should
					// perform a server request. When it returns, it would store it and next time that ModID
					// would be retrieable.
					return {};
				}

				ModElems.Append(OpModInfo.value());
			}

			if (ModElems.Size() <= 0) 
			{ 
				// In case ModElems does not have any ModInfo, it should return the "empty" from optional
				return {};
			}

			return ModElems;
		}

		void CacheService::ClearCache()
		{
			CacheInstance.reset(new Cache());
		}
	} // namespace Detail
} // namespace Modio
