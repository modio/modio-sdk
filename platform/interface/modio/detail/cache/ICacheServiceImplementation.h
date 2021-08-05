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