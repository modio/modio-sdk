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

#include "modio/core/ModioServices.h"
#include "modio/detail/ops/DownloadFileOp.h"
#include "modio/detail/ops/compression/ExtractAllToFolderOp.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/http/ModioHttpService.h"
#include "modio/detail/AsioWrapper.h"

namespace Modio
{
	namespace Detail
	{
		namespace ComposedOps
		{
			
			/// @brief Begins an asynchronous operation that performs a HTTP request and buffers the entire response into memory
			/// @tparam CompletionTokenType Type of the completion handler for the operation
			/// @param Response Dynamic buffer to hold the response data
			/// @param RequestParameters Request Parameters
			/// @param AllowCachedResponse Should the cache be checked for this request or do we want to always hit the server
			/// @param Token Concrete completion handler instance
			template<typename CompletionTokenType>
			auto PerformRequestAndGetResponseAsync(Modio::Detail::DynamicBuffer Response,
													Modio::Detail::HttpRequestParams RequestParameters,
													Modio::Detail::CachedResponse AllowCachedResponse,
													CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
					PerformRequestAndGetResponseOp(
						Response, RequestParameters,
						Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetAPIRequestTicket(),
						AllowCachedResponse),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto DownloadFileAsync(Modio::Detail::HttpRequestParams DownloadParameters,
									Modio::filesystem::path DestinationPath,
									Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ModProgress,
									CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
					DownloadFileOp(
						DownloadParameters, DestinationPath,
						Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetFileDownloadTicket(),
						ModProgress),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto DownloadImageAsync(Modio::Detail::HttpRequestParams DownloadParameters,
									 Modio::filesystem::path DestinationPath, CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
					DownloadFileOp(
						DownloadParameters, DestinationPath,
						Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetAPIRequestTicket(),
						{}),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		} // namespace ComposedOps
	} // namespace Detail
} // namespace Modio
