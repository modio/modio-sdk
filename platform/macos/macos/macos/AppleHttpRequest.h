/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioErrorCode.h"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Modio
{
	namespace Detail
	{
		namespace Apple
		{
			class HttpSession;

			// Wraps an NSURLSessionDataTask or NSURLSessionUploadTask.
			// Thread-safe between the SDK polling thread and the NSURLSession delegate queue.
			//
			// State flow: Pending -> InFlight -> HeadersReceived -> Complete/Failed/Cancelled
			class HttpRequest
			{
			public:
				~HttpRequest();
				HttpRequest(const HttpRequest&) = delete;
				HttpRequest& operator=(const HttpRequest&) = delete;

				// Setup (before Start)

				void SetURL(const std::string& URL);
				void SetVerb(const std::string& Verb);
				void AddHeader(const std::string& Name, const std::string& Value);

				// Sets an in-memory request body. Mutually exclusive with BeginStreamedBody.
				void SetBody(Modio::Detail::Buffer Body);

				// Prepares for incremental upload via WriteRequestBody.
				// Mutually exclusive with SetBody.
				void BeginStreamedBody(std::uint64_t TotalLength);

				// Lifecycle

				void Start();
				void Cancel();

				// Poll state

				enum class State
				{
					Pending,
					InFlight,
					HeadersReceived,
					Complete,
					Failed,
					Cancelled,
				};

				State GetState() const;

				// Valid when GetState() == Failed.
				Modio::ErrorCode GetError() const;

				// Response headers (valid once GetState() >= HeadersReceived)

				std::uint32_t GetResponseCode() const;
				std::vector<std::pair<std::string, std::string>> GetResponseHeaders() const;

				// Response body

				// Drains up to MaxBytes from the receive buffer. Returns bytes drained.
				std::size_t DrainResponseBody(std::uint8_t* Out, std::size_t MaxBytes);
				std::size_t DrainResponseBody(Modio::Detail::DynamicBuffer& Out, std::size_t MaxBytes = 0);
				bool HasBufferedBody() const;

				// Streaming body upload

				// Pushes bytes into the upload stream. May accept fewer than Length
				// if the internal buffer is full; retry next tick.
				std::size_t WriteRequestBody(const std::uint8_t* Bytes, std::size_t Length);
				std::size_t WriteRequestBody(const Modio::Detail::Buffer& Data);
				bool CanAcceptBodyBytes() const;

				// Public so the Obj-C delegate can name the type.
				// Full definition in AppleHttp_Internal.h.
				struct Impl;

			private:
				friend class HttpSession;
				explicit HttpRequest(HttpSession& Owner);

				// shared_ptr so delegate callbacks can pin Impl via weak_ptr.
				std::shared_ptr<Impl> PImpl;
			};
		} // namespace Apple
	} // namespace Detail
} // namespace Modio