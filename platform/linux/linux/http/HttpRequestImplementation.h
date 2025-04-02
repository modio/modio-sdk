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

#include "httpparser/response.h"
#include "modio/detail/MbedtlsWrapper.h"
#include "modio/detail/ModioStringHelpers.h"
#include "modio/detail/http/IHttpRequestImplementation.h"
#include "modio/http/ModioHttpParams.h"

struct HttpRequestImplementation : public Modio::Detail::IHttpRequestImplementation
{
	std::uint32_t ResponseCode = 0;
	mbedtls_net_context Socket;
	mbedtls_ssl_context SSLContext;
	/// @brief Temporary buffer for response body data to enable us to handle chunked encoding transparently
	Modio::Detail::DynamicBuffer ResponseDataBuffer;

	httpparser::Response ParsedResponseHeaders;
	std::size_t ResponseBodyReceivedLength = 0;
	std::size_t CurrentChunkSizeRemaining = 0;
	Modio::Optional<std::size_t> GetContentLength()
	{
		Modio::Optional<std::string> Res = GetHeaderValue("Content-Length");
		if (Res.has_value())
		{
			return std::stoull(Res.value());
		}
		
		return {};
	}

	virtual ~HttpRequestImplementation() {}
	// Common members
	Modio::Detail::HttpRequestParams Parameters;
	bool HasBeenSent()
	{
		return false;
	}

	std::uint32_t GetResponseCode() override
	{
		return ResponseCode;
	}

	virtual Modio::Detail::HttpRequestParams& GetParameters() override
	{
		return Parameters;
	}

	virtual Modio::Optional<std::string> GetRedirectURL() override
	{
		return GetHeaderValue("location");
	}

	virtual Modio::Optional<std::uint32_t> GetRetryAfter() override
	{
		Modio::Optional<std::string> Res = GetHeaderValue("Retry-After");
		if (Res.has_value())
		{
			return Modio::Detail::String::ParseDateOrInt(Res.value());
		}
		
		return {};
	}

	virtual Modio::Optional<std::string> GetHeaderValue(std::string HeaderKey) override
	{
		for (httpparser::Response::HeaderItem& Hdr : ParsedResponseHeaders.headers)
		{
			if (Modio::Detail::String::MatchesCaseInsensitive(Hdr.name, HeaderKey))
			{
				return Hdr.value;
			}
		}

		return {};
	}
};
