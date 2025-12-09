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
	#include "modio/http/ModioHttpRequest.h"
#endif

#include "modio/core/ModioLogger.h"

namespace Modio
{
	namespace Detail
	{
		HttpRequest::HttpRequest(HttpRequest&& Other)
			: ModioAsio::basic_io_object<HttpService>(std::move(Other)),
			  RequestParameters(std::move(Other.RequestParameters))

		{}

		HttpRequest::HttpRequest(ModioAsio::io_context& Context, HttpRequestParams RequestParams)
			: ModioAsio::basic_io_object<HttpService>(Context),
			  RequestParameters(RequestParams)

		{
			get_implementation()->Parameters = RequestParams;
			Logger().Log(LogLevel::Trace, LogCategory::Http, "Creating Request for {}{}",
						 RequestParams.GetServerAddress(), RequestParams.GetFormattedResourcePath());
		}

		HttpRequest::HttpRequest(HttpRequestParams RequestParams)
			: ModioAsio::basic_io_object<HttpService>(Modio::Detail::Services::GetGlobalContext()),
			  RequestParameters(RequestParams)
		{
			get_implementation()->Parameters = RequestParameters;
			Logger().Log(LogLevel::Trace, LogCategory::Http, "Creating Request for {}{}",
						 RequestParams.GetServerAddress(), RequestParams.GetFormattedResourcePath());
		}

		HttpRequest::~HttpRequest() {}

		Modio::Detail::HttpRequestParams& HttpRequest::Parameters()
		{
			return get_implementation()->Parameters;
		}

		std::uint32_t HttpRequest::GetResponseCode()
		{
			return get_implementation()->GetResponseCode();
		}

		Modio::Optional<std::string> HttpRequest::GetRedirectURL() const 
		{
			return get_implementation()->GetRedirectURL();
		}

		Modio::Optional<std::uint32_t> HttpRequest::GetRetryAfter()
		{
			return get_implementation()->GetRetryAfter();
		}

		Modio::Optional<std::string> HttpRequest::GetHeaderValue(std::string HeaderKey)
		{
			return get_implementation()->GetHeaderValue(HeaderKey);
		}
	} // namespace Detail
} // namespace Modio
