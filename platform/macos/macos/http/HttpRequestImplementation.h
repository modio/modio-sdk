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
#include "modio/detail/ModioStringHelpers.h"
#include "modio/detail/http/IHttpRequestImplementation.h"
#include "modio/http/ModioHttpParams.h"

#include <CFNetwork/CFNetwork.h>
#include <CoreFoundation/CoreFoundation.h>

struct HttpRequestImplementation : public Modio::Detail::IHttpRequestImplementation
{
	std::uint32_t ResponseCode = 0;
	/// @brief Temporary buffer for response body data to enable us to handle chunked encoding transparently
	Modio::Detail::DynamicBuffer ResponseDataBuffer;
	httpparser::Response ParsedResponseHeaders;
	std::size_t ResponseBodyReceivedLength = 0;
	std::size_t CurrentChunkSizeRemaining = 0;
	Modio::Detail::HttpRequestParams Parameters;

	CFReadStreamRef ReadStream;
	CFWriteStreamRef WriteStream;
	CFDataRef HTTPRequestData;

	Modio::Optional<std::size_t> GetContentLength()
	{
		for (httpparser::Response::HeaderItem& Hdr : ParsedResponseHeaders.headers)
		{
			if (Modio::Detail::String::MatchesCaseInsensitive(Hdr.name,"Content-Length"))
			{
				return std::stoull(Hdr.value);
			}
		}

		return {};
	}

	virtual ~HttpRequestImplementation()
	{
		if (ReadStream != NULL)
		{
			CFReadStreamClose(ReadStream);
			CFRelease(ReadStream);
			ReadStream = NULL;
		}

		if (WriteStream != NULL)
		{
			CFWriteStreamClose(WriteStream);
			CFRelease(WriteStream);
			WriteStream = NULL;
		}
	}

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
		for (httpparser::Response::HeaderItem& Hdr : ParsedResponseHeaders.headers)
		{
			if (Modio::Detail::String::MatchesCaseInsensitive(Hdr.name,"location"))
			{
				return Hdr.value;
			}
		}

		return {};
	}

	// A boolean to signal if the stream has bytes available to read
	bool ReadStreamHasBytes()
	{
		if (ReadStream == NULL)
		{
			return false;
		}

		return CFReadStreamHasBytesAvailable(ReadStream);
	}

	CFStreamStatus ReadStreamStatus()
	{
		if (ReadStream == NULL)
		{
			return kCFStreamStatusNotOpen;
		}

		return CFReadStreamGetStatus(ReadStream);
	}

	bool WriteStreamAcceptsBytes()
	{
		if (WriteStream == NULL)
		{
			return false;
		}

		return CFWriteStreamCanAcceptBytes(WriteStream);
	}

	CFStreamStatus WriteStreamStatus()
	{
		if (WriteStream == NULL)
		{
			return kCFStreamStatusNotOpen;
		}

		return CFWriteStreamGetStatus(WriteStream);
	}
};
