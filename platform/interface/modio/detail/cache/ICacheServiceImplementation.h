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

namespace Modio
{
	namespace Detail
	{
		class ICacheServiceImplementation
		{
		public:
			virtual void SetCacheExpireTime(std::chrono::steady_clock::duration ExpireTime) = 0;
			virtual void AddToCache(std::string ResourceURL, Modio::Detail::DynamicBuffer ResponseData) = 0;
			virtual void Modio::Optional<Modio::Detail::DynamicBuffer> FetchFromCache(std::string ResourceURL) = 0;
			virtual void ClearCache() = 0;
		};
	}
}