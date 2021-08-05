#pragma once
#include "modio/core/ModioStdTypes.h"
#include "modio/http/ModioHttpParams.h"
#include <string>

namespace Modio
{
	namespace Detail
	{
		class IHttpRequestImplementation
		{
		public:
			virtual Modio::Detail::HttpRequestParams& GetParameters() = 0;
			virtual std::uint32_t GetResponseCode() = 0;
			virtual Modio::Optional<std::string> GetRedirectURL() = 0;
		};
	} // namespace Detail
} // namespace Modio