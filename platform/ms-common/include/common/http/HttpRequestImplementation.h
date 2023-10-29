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
#include "modio/detail/http/IHttpRequestImplementation.h"
#include "modio/http/ModioHttpParams.h"
#include "common/HttpSharedState.h"

class HttpRequestImplementation : public Modio::Detail::IHttpRequestImplementation
{
public:
	MODIO_IMPL virtual ~HttpRequestImplementation();

	HINTERNET ConnectionHandle;
	HINTERNET RequestHandle;
	std::uint32_t ResponseCode;
	Modio::Optional<std::uint32_t> RetryAfter;
	Modio::Detail::HttpRequestParams Parameters;
	std::map<std::string, std::string> ResponseHeaders;

	MODIO_IMPL bool HasBeenSent();

	// Common members
	MODIO_IMPL virtual std::uint32_t GetResponseCode() override;

	MODIO_IMPL virtual Modio::Detail::HttpRequestParams& GetParameters() override;
	
	MODIO_IMPL virtual Modio::Optional<std::string> GetRedirectURL() override;
	
	MODIO_IMPL virtual Modio::Optional<std::uint32_t> GetRetryAfter() override;

	MODIO_IMPL virtual Modio::Optional<std::string> GetHeaderValue(std::string Key) override;
};


#ifndef MODIO_SEPARATE_COMPILATION
#include "HttpRequestImplementation.ipp"
#endif
