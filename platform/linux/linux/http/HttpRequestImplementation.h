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
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
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
		for (httpparser::Response::HeaderItem& Hdr : ParsedResponseHeaders.headers)
		{
			if (Hdr.name.compare("Content-Length") == 0)
			{
				return std::stoull(Hdr.value);
			}
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

	std::uint32_t GetResponseCode()
	{
		return ResponseCode;
	}

	virtual Modio::Detail::HttpRequestParams& GetParameters() override
	{
		return Parameters;
	}
	virtual Modio::Optional<std::string> GetRedirectURL() override
	{
		for (httpparser::Response::HeaderItem& Hdr : ParsedResponseHeaders.headers)
		{
			if (Hdr.name.compare("location") == 0)
			{
				return Hdr.value;
			}
		}
		
		return {};
	}
};
