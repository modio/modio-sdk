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
#include "macos/AppleHttpRequest.h"
#include "modio/detail/ModioStringHelpers.h"
#include "modio/detail/http/IHttpRequestImplementation.h"
#include "modio/http/ModioHttpParams.h"

#include <memory>

struct HttpRequestImplementation : public Modio::Detail::IHttpRequestImplementation
{
	std::uint32_t ResponseCode = 0;
	httpparser::Response ParsedResponseHeaders {};
	std::size_t ResponseBodyReceivedLength = 0;
	Modio::Detail::HttpRequestParams Parameters {};

	// Created by HttpSharedState::InitializeRequest. Holds the NSURLSessionTask
	// and all Obj-C state. Destructor cancels the task if it's still running.
	std::unique_ptr<Modio::Detail::Apple::HttpRequest> AppleRequest;

	Modio::Optional<std::size_t> GetContentLength()
	{
		Modio::Optional<std::string> Res = GetHeaderValue("Content-Length");
		if (Res.has_value())
		{
			return std::stoull(Res.value());
		}
		return {};
	}

	virtual ~HttpRequestImplementation() = default;

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

	virtual std::vector<std::pair<std::string, std::string>> GetAllHeaders() override
	{
		std::vector<std::pair<std::string, std::string>> Result;
		for (const httpparser::Response::HeaderItem& Hdr : ParsedResponseHeaders.headers)
		{
			Result.emplace_back(Hdr.name, Hdr.value);
		}
		return Result;
	}

};
