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

#include "ModioGeneratedVariables.h"
#include "modio/core/ModioBuffer.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/http/ModioHttpService.h"

namespace Modio
{
	namespace Detail
	{
		class HttpRequest : public asio::basic_io_object<HttpService>
		{
			HttpRequestParams RequestParameters;

		public:
			MODIO_IMPL explicit HttpRequest(HttpRequestParams RequestParams);

			MODIO_IMPL explicit HttpRequest(asio::io_context& Context, HttpRequestParams RequestParams);

			MODIO_IMPL HttpRequest(HttpRequest&& Other);
			MODIO_IMPL ~HttpRequest();
			MODIO_IMPL HttpRequestParams& Parameters();

			MODIO_IMPL std::uint32_t GetResponseCode();

			MODIO_IMPL Modio::Optional<std::string> GetRedirectURL() const;

			/// @brief Sends a request to the server. Assumes that any payload data is stored in its entirety on this
			/// HttpRequest already - if you want to stream data to the server please use
			/// BeginWriteAsync/WriteSomeAsync.
			/// @tparam CompletionTokenType Deduced type of the token/Callable being used as a callback
			/// @param Token Callable with signature void(Modio::ErrorCode)
			template<typename CompletionTokenType>
			auto SendAsync(CompletionTokenType&& Token)
			{
				// TODO: @Modio-core Double check if this is a pessimizing move
				// TODO: @Modio-core prevent double-sending
				get_service().SendRequestAsync(get_implementation(),
											   std::forward<CompletionTokenType>(std::move(Token)));
			}

			/// @brief Begins the send operation for a request which requires a streamed upload
			/// @tparam CompletionTokenType Deduced type of the token/callable being used as a callback
			/// @param TotalSize The total amount of data which will be streamed
			/// @param Token Callable with signature void(Modio::ErrorCode)
			template<typename CompletionTokenType>
			auto BeginWriteAsync(Modio::FileSize TotalSize, CompletionTokenType&& Token)
			{
				get_service().BeginWriteAsync(get_implementation(), TotalSize,
											  std::forward<CompletionTokenType>(Token));
			}

			/// @brief Uploads the passed-in buffer as part of a streamed upload to the server. Call
			/// ReadResponseHeadersAsync when you are done uploading data.
			/// @tparam CompletionTokenType Deduced type of the token/callable being used as a callback
			/// @param Data Buffer containing the data to submit
			/// @param Token Callable with signature void(Modio::ErrorCode)
			template<typename CompletionTokenType>
			auto WriteSomeAsync(Modio::Detail::Buffer Data, CompletionTokenType&& Token)
			{
				get_service().WriteSomeAsync(get_implementation(), std::move(Data),
											 std::forward<CompletionTokenType>(Token));
			}

			template<typename CompletionTokenType>
			auto ReadResponseHeadersAsync(CompletionTokenType&& Token)
			{
				// Must have sent the request first before we read headers

				get_service().ReadResponseHeadersAsync(get_implementation(),
													   std::forward<CompletionTokenType>(std::move(Token)));
			}

			// pass in a mutable buffer to read into as well?
			template<typename CompletionTokenType>
			auto ReadSomeFromResponseBodyAsync(DynamicBuffer DynamicBufferInstance, CompletionTokenType&& Token)
			{
				get_service().ReadSomeFromResponseBodyAsync(get_implementation(), DynamicBufferInstance,
															std::forward<CompletionTokenType>(std::move(Token)));
			}
		};
	} // namespace Detail
}; // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioHttpRequest.ipp"
#endif