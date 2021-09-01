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
#include "modio/http/ModioHttpParams.h"
#include "modio/http/ModioHttpService.h"
#include "modio/detail/AsioWrapper.h"

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

			// pass in a const buffer as well?
			template<typename CompletionTokenType>
			auto SendAsync(CompletionTokenType&& Token)
			{
				// TODO: @Modio-core Make sure request hasn't been sent already
				get_service().SendRequestAsync(get_implementation(),
												std::forward<CompletionTokenType>(std::move(Token)));
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