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
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioGameInfo.h"
#include "modio/core/entities/ModioModInfo.h"
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/core/entities/ModioModCollection.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/timer/ModioTimer.h"
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>

namespace Modio
{
	namespace Detail
	{
		class DynamicBuffer;

		/// @docinternal
		/// @brief A global service to store mod data retrieved from the server
		/// in a form of cache, which would have a limited lifespan
		class CacheService : public asio::detail::service_base<CacheService>
		{
		public:
			MODIO_IMPL explicit CacheService(asio::io_context& IOService);
			CacheService(CacheService&&) = delete;

			MODIO_IMPL void Shutdown();

			using implementation_type = std::uint32_t;

			MODIO_IMPL void construct(implementation_type& Implementation);

			MODIO_IMPL void destroy(implementation_type& Implementation);

			MODIO_IMPL void SetCacheExpireTime(std::chrono::steady_clock::duration ExpireTime);

			MODIO_IMPL void AddToCache(std::string ResourceURL, class Modio::Detail::DynamicBuffer ResponseData);

			MODIO_IMPL void AddToCache(Modio::ModInfo ModInfoDetail);

			MODIO_IMPL void AddToCache(Modio::ModCollectionInfo ModCollectionInfoDetail);

			MODIO_IMPL void AddToCache(Modio::GameInfo GameInfoDetails);

			MODIO_IMPL void AddToCache(Modio::GameID GameIDDetail, Modio::ModInfoList ModInfoDetails);

			MODIO_IMPL void AddToCache(Modio::GameID GameIDDetail, Modio::ModCollectionInfoList ModCollectionInfoDetails);

			MODIO_IMPL void AddToCache(Modio::ModID ModId, std::uint64_t Filesize, bool recursive);

			MODIO_IMPL List<std::vector, Modio::ModID> GetAllModIdsInCache();

			MODIO_IMPL Modio::Optional<Modio::Detail::DynamicBuffer> FetchFromCache(std::string ResourceURL) const;

			MODIO_IMPL Modio::Optional<Modio::ModInfo> FetchFromCache(Modio::ModID ModIDDetail) const;

			MODIO_IMPL Modio::Optional<Modio::ModCollectionInfo> FetchFromCache(Modio::ModCollectionID ModCollectionIDDetail) const;

			MODIO_IMPL Modio::Optional<Modio::GameInfo> FetchGameInfoFromCache(Modio::GameID GameIDDetail) const;

			MODIO_IMPL Modio::Optional<Modio::ModInfoList> FetchFromCache(Modio::GameID GameIDDetail) const;

			MODIO_IMPL Modio::Optional<std::uint64_t> FetchModDependencyFilesizeFromCache(Modio::ModID ModIDDetail,
																						  bool recursive) const;

			MODIO_IMPL void ClearCache();

		private:
			struct CacheEntry
			{
				Modio::Detail::Timer MyTimer;
				// std::unique_ptr<asio::steady_timer> Timer;
				Modio::Detail::DynamicBuffer Data;
			};

			struct ModDependencyFilesizeEntry
			{
				// The filesize calculated of the immediate child dependencies
				std::uint64_t FilesizeFirstDepth = 0;
				// The filesize calculated from a recursive request of dependencies
				std::uint64_t FilesizeRecursive = 0;
			};

			struct Cache
			{
				std::unordered_map<std::uint32_t, CacheEntry> CacheEntries;
				std::unordered_map<std::int64_t, Modio::ModInfo> ModInfoCache;
				std::unordered_map<std::int64_t, Modio::ModCollectionInfo> ModCollectionInfoCache;
				std::unordered_map<std::int64_t, Modio::GameInfo> GameInfoCache;
				std::unordered_map<std::int64_t, std::vector<Modio::ModID>> ModInfoListCache;
				std::unordered_map<std::int64_t, std::vector<Modio::ModCollectionID>> ModCollectionInfoListCache;
				std::unordered_map<std::int64_t, ModDependencyFilesizeEntry> ModDependenciesFilesize;
			};

			std::shared_ptr<Cache> CacheInstance;
			std::chrono::steady_clock::duration CacheExpiryTime = std::chrono::seconds(15);
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioCacheService.ipp"
#endif