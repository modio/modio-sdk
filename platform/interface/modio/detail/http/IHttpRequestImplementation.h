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
			virtual ~IHttpRequestImplementation() {}
			virtual Modio::Detail::HttpRequestParams& GetParameters() = 0;
			virtual std::uint32_t GetResponseCode() = 0;
			virtual Modio::Optional<std::string> GetRedirectURL() = 0;
			virtual Modio::Optional<std::uint32_t> GetRetryAfter() = 0;
			virtual Modio::Optional<std::string> GetHeaderValue(std::string Key) = 0;
		};
	} // namespace Detail
} // namespace Modio
