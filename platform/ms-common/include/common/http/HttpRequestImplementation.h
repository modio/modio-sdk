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
#include "winhttp.h"

struct HttpRequestImplementation : public Modio::Detail::IHttpRequestImplementation
{
	HINTERNET ConnectionHandle;
	HINTERNET RequestHandle;
	std::uint32_t ResponseCode;

	virtual ~HttpRequestImplementation()
	{
		WinHttpCloseHandle(RequestHandle);
		WinHttpCloseHandle(ConnectionHandle);
	}
	// Common members
	Modio::Detail::HttpRequestParams Parameters;
	bool HasBeenSent()
	{
		return ConnectionHandle != nullptr;
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
		return {};
	}
};
