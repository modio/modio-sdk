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
#include "ModioGeneratedVariables.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/entities/ModioModInfo.h"
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/detail/AsioWrapper.h"
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>

namespace Modio
{
	
	namespace Detail
	{
		class DynamicBuffer;

		class CacheService : public asio::detail::service_base<CacheService>
		{
		public:
			MODIO_IMPL explicit CacheService(asio::io_context& IOService);

			MODIO_IMPL void Shutdown();

			using implementation_type = std::uint32_t;

			MODIO_IMPL void construct(implementation_type& Implementation);

			MODIO_IMPL void destroy(implementation_type& Implementation);

			MODIO_IMPL void SetCacheExpireTime(std::chrono::steady_clock::duration ExpireTime);

			MODIO_IMPL void AddToCache(std::string ResourceURL, class Modio::Detail::DynamicBuffer ResponseData);

			MODIO_IMPL void AddToCache(Modio::ModInfo ModInfoDetail);

			MODIO_IMPL void AddToCache(Modio::GameID GameIDDetail, Modio::ModInfoList ModInfoDetails);

			MODIO_IMPL Modio::Optional<Modio::Detail::DynamicBuffer> FetchFromCache(std::string ResourceURL) const;

			MODIO_IMPL Modio::Optional<Modio::ModInfo> FetchFromCache(Modio::ModID ModIDDetail) const;

			MODIO_IMPL Modio::Optional<Modio::ModInfoList> FetchFromCache(Modio::GameID GameIDDetail) const;

			MODIO_IMPL void ClearCache();

		private:
			struct CacheEntry
			{
				std::unique_ptr<asio::steady_timer> Timer;
				Modio::Detail::DynamicBuffer Data;
			};

			struct Cache
			{
				std::unordered_map<std::uint32_t, CacheEntry> CacheEntries;
				std::unordered_map<std::int64_t, Modio::ModInfo> ModInfoCache;
				std::unordered_map<std::int64_t, std::vector<Modio::ModID>> ModInfoListCache;
			};

			std::shared_ptr<Cache> CacheInstance;
			std::chrono::steady_clock::duration CacheExpiryTime = std::chrono::seconds(15);
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioCacheService.ipp"
#endif