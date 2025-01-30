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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogEnum.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ModioSDKSessionData.h"

namespace Modio
{
	namespace Detail
	{
		CacheService::CacheService(asio::io_context& IOService) : asio::detail::service_base<CacheService>(IOService)
		{
			CacheInstance = std::make_shared<Cache>();
		}

		void CacheService::Shutdown()
		{
			// Cancel all timers
			for (auto& CacheEntry : CacheInstance->CacheEntries)
			{
				CacheEntry.second.MyTimer.Cancel();
			}
			ClearCache();
		}

		void CacheService::construct(implementation_type& MODIO_UNUSED_ARGUMENT(Implementation)) {}

		void CacheService::destroy(implementation_type& MODIO_UNUSED_ARGUMENT(Implementation)) {}

		void CacheService::SetCacheExpireTime(std::chrono::steady_clock::duration ExpireTime)
		{
			CacheExpiryTime = ExpireTime;
		}

		void CacheService::AddToCache(std::string ResourceURL, Modio::Detail::DynamicBuffer ResponseData)
		{
			MODIO_PROFILE_SCOPE(CacheAddURL);
			auto Hasher = std::hash<std::string>();
			std::uint32_t URLHash = std::uint32_t(Hasher(ResourceURL));

			// Don't add the instance again as if the instance is already in there, then we will destroy the old timer
			// and it will expire causing unnecessary removals
			if (CacheInstance->CacheEntries.find(URLHash) == std::end(CacheInstance->CacheEntries))
			{
				// @todo-optimize This will fragment the heap quite much, rewrite using another container so we don't
				// need to allocate so many objects on the heap
				Modio::Detail::Timer CacheExpiryTimer;
				CacheExpiryTimer.ExpiresAfter(CacheExpiryTime);

				Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http, "Adding hash {} to cache", URLHash);

				auto DeleteCacheEntry = [WeakCacheReference = std::weak_ptr<Cache>(CacheInstance),
										 URLHash](std::error_code) mutable {
					std::shared_ptr<Cache> CacheReference = WeakCacheReference.lock();
					Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http, "Removing hash {} from cache",
												URLHash);
					if (CacheReference)
					{
						CacheReference->CacheEntries.erase(URLHash);
					}
				};

				CacheExpiryTimer.WaitAsync(std::move(DeleteCacheEntry));

				CacheEntry Entry = {std::move(CacheExpiryTimer), std::move(ResponseData)};
				CacheInstance->CacheEntries.emplace(URLHash, std::move(Entry));
			}
		}

		void CacheService::AddToCache(Modio::ModInfo ModInfoDetails)
		{
			Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http, "Adding ModID {} to cache",
										ModInfoDetails.ModId);
			// The ModInfoCache would clean only when the mod.io SDK session ends. For that reason there is no
			// timer for this case. Another way to remove this is by calling "ClearCache"
			CacheInstance->ModInfoCache.insert_or_assign(ModInfoDetails.ModId, ModInfoDetails);
		}

		void CacheService::AddToCache(Modio::GameInfo GameInfoDetails)
		{
			Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http, "Adding GameID {} to cache",
										GameInfoDetails.GameID);

			CacheInstance->GameInfoCache.insert_or_assign(GameInfoDetails.GameID, GameInfoDetails);
		}

		void CacheService::AddToCache(Modio::GameID GameIDDetail, Modio::ModInfoList ModInfoDetails)
		{
			Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http, "Adding ModIDList to cache with GameID: {}",
										GameIDDetail);
			std::vector<Modio::ModID> ModIDVec;

			// Instead of keeping the whole list (with possible data replications), it adds single elements to the
			// cache.
			for_each(ModInfoDetails.begin(), ModInfoDetails.end(), [this, &ModIDVec](Modio::ModInfo ModInfoData) {
				this->AddToCache(ModInfoData);
				ModIDVec.push_back(ModInfoData.ModId);
			});

			CacheInstance->ModInfoListCache.emplace(GameIDDetail, ModIDVec);
		}

		void CacheService::AddToCache(Modio::ModID ModId, std::uint64_t Filesize, bool recursive)
		{
			Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http, "Adding ModID {} filesize to cache = {}",
										ModId, Filesize);

			// See if we already have this mod cached, and then set the appropriate filesize depending on the recursive
			// flag
			ModDependencyFilesizeEntry Entry;
			auto CacheEntryIterator = CacheInstance->ModDependenciesFilesize.find(ModId);
			if (CacheEntryIterator != CacheInstance->ModDependenciesFilesize.end())
			{
				Entry = std::move(CacheEntryIterator->second);
			}

			if (recursive)
			{
				Entry.FilesizeRecursive = Filesize;
			}
			else
			{
				Entry.FilesizeFirstDepth = Filesize;
			}

			CacheInstance->ModDependenciesFilesize.emplace(ModId, std::move(Entry));
		}

		List<std::vector, Modio::ModID> CacheService::GetAllModIdsInCache()
		{
			List<std::vector, Modio::ModID> listModId;

			// Get ModIds from primary cache
			for (auto& CacheEntry : CacheInstance->ModInfoCache)
			{
				listModId.GetRawList().push_back(CacheEntry.second.ModId);
			}

			// Get ModIds from secondary cache
			for (auto ModEntry : Modio::Detail::SDKSessionData::GetSystemModCollection().Entries())
			{
				if (std::find(listModId.GetRawList().begin(), listModId.GetRawList().end(), ModEntry.first) !=
					listModId.GetRawList().end())
				{
					listModId.GetRawList().push_back(ModEntry.first);
				}
			}

			return listModId;
		}

		Modio::Optional<Modio::Detail::DynamicBuffer> CacheService::FetchFromCache(std::string ResourceURL) const
		{
			MODIO_PROFILE_SCOPE(CacheFetchURL);
			auto Hasher = std::hash<std::string>();
			std::uint32_t URLHash = std::uint32_t(Hasher(ResourceURL));

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
			MODIO_PROFILE_SCOPE(CacheFetchMod);
			auto CacheEntryIterator = CacheInstance->ModInfoCache.find(ModIDDetail);
			if (CacheEntryIterator != CacheInstance->ModInfoCache.end())
			{
				Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http, "Retrieving mod {} from primary cache",
											ModIDDetail);

				if (Modio::Detail::SDKSessionData::IsModCacheInvalid(ModIDDetail) == true)
				{
					return {};
				}

				return CacheEntryIterator->second;
			}

			Modio::Optional<Modio::ModCollectionEntry&> CachedModInfo =
				Modio::Detail::SDKSessionData::GetSystemModCollection().GetByModID(ModIDDetail);
			if (CachedModInfo.has_value())
			{
				Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http,
											"Retrieving mod {} from secondary cache", ModIDDetail);
				return CachedModInfo->GetModProfile();
			}

			return {};
		}

		Modio::Optional<Modio::GameInfo> CacheService::FetchGameInfoFromCache(Modio::GameID GameIDDetail) const
		{
			auto CacheEntryIterator = CacheInstance->GameInfoCache.find(GameIDDetail);
			if (CacheEntryIterator != CacheInstance->GameInfoCache.end())
			{
				Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http, "Retrieving game {} from primary cache",
											GameIDDetail);
				return CacheEntryIterator->second;
			}
			return {};
		}

		Modio::Optional<Modio::ModInfoList> CacheService::FetchFromCache(Modio::GameID GameIDDetails) const
		{
			MODIO_PROFILE_SCOPE(CacheFetchGame);
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

		Modio::Optional<std::uint64_t> CacheService::FetchModDependencyFilesizeFromCache(Modio::ModID ModIDDetail,
																						 bool recursive) const
		{
			MODIO_PROFILE_SCOPE(CacheFetchModDependencyFilesize);
			auto CacheEntryIterator = CacheInstance->ModDependenciesFilesize.find(ModIDDetail);
			if (CacheEntryIterator != CacheInstance->ModDependenciesFilesize.end())
			{
				Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::Http,
											"Retrieving mod {} dependency filesize from primary cache", ModIDDetail);

				if (Modio::Detail::SDKSessionData::IsModCacheInvalid(ModIDDetail) == true)
				{
					return {};
				}

				// Grab the appropriate cache value based on the recursive flag
				std::uint64_t Filesize = 0;
				if (recursive)
				{
					Filesize = CacheEntryIterator->second.FilesizeRecursive;
				}
				else
				{
					Filesize = CacheEntryIterator->second.FilesizeFirstDepth;
				}

				// If we haven't cached a filesize for this mod, it should return the "empty" from optional
				if (Filesize == 0)
				{
					return {};
				}
			}

			return {};
		}

		void CacheService::ClearCache()
		{
			CacheInstance.reset(new Cache());
		}
	} // namespace Detail
} // namespace Modio
