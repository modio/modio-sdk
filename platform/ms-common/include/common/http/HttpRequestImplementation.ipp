/*
 *  Copyright (C) 2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#ifdef MODIO_SEPARATE_COMPILATION
	#include "common/http/HttpRequestImplementation.h"
#endif

HttpRequestImplementation::~HttpRequestImplementation()
{
	std::shared_ptr<HttpSharedStateBase> SharedState = SharedStateHolder::Get().SharedStatePtr.lock();
	if (SharedState)
	{
		SharedState->EraseCabllbackStatus(RequestHandle);
	}
	if (RequestHandle != nullptr)
	{
		WinHttpCloseHandle(RequestHandle);
	}
	if (ConnectionHandle != nullptr)
	{
		WinHttpCloseHandle(ConnectionHandle);
	}
}

std::uint32_t HttpRequestImplementation::GetResponseCode()
{
	return ResponseCode;
}

bool HttpRequestImplementation::HasBeenSent()
{
	return ConnectionHandle != nullptr;
}

Modio::Detail::HttpRequestParams& HttpRequestImplementation::GetParameters()
{
	return Parameters;
}

Modio::Optional<std::string> HttpRequestImplementation::GetRedirectURL()
{
	return {};
}

Modio::Optional<std::uint32_t> HttpRequestImplementation::GetRetryAfter()
{
	Modio::Optional<std::string> Res = GetHeaderValue("Retry-After");
	if (Res.has_value())
	{
		return Modio::Detail::String::ParseDateOrInt(Res.value());
	}

	return {};
}

Modio::Optional<std::string> HttpRequestImplementation::GetHeaderValue(std::string Key)
{
	auto Position = ResponseHeaders.find(Key);
	if (Position != ResponseHeaders.end())
	{
		return Position->second;
	}

	return {};
}
