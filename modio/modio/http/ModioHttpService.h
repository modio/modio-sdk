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
#include "http/HttpImplementation.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioOperationQueue.h"
#include "modio/detail/ModioSDKSessionData.h"
#include <iostream>
#include <memory>

namespace Modio
{
	namespace Detail
	{
		class HttpService : public asio::detail::service_base<HttpService>
		{
			std::shared_ptr<HttpImplementation> PlatformImplementation;
			// Using shared_ptr here because queue tickets observe the queue
			std::shared_ptr<Modio::Detail::OperationQueue> APIQueue;
			std::shared_ptr<Modio::Detail::OperationQueue> FileDownloadQueue;

		public:
			MODIO_IMPL explicit HttpService(asio::io_context& IOService);

			using implementation_type = HttpImplementation::IOObjectImplementationType;

			MODIO_IMPL void construct(implementation_type& Implementation);

			MODIO_IMPL void move_construct(implementation_type& Implementation, implementation_type& Other);

			MODIO_IMPL void move_assign(implementation_type& Implementation, Modio::Detail::HttpService& OtherService,
										implementation_type& Other);

			MODIO_IMPL void converting_move_construct(implementation_type& impl, Modio::Detail::HttpService&,
													  implementation_type& other_impl);

			MODIO_IMPL void converting_move_assign(implementation_type& impl, Modio::Detail::HttpService& other_service,
												   implementation_type& other_impl);

			MODIO_IMPL void destroy(implementation_type& Implementation);

			template<typename CompletionHandler>
			auto InitializeAsync(CompletionHandler Handler)
			{
				return PlatformImplementation->InitializeHTTPAsync(std::forward<CompletionHandler>(std::move(Handler)));
			}

			template<typename CompletionHandler>
			auto SendRequestAsync(implementation_type& PlatformIOObject, CompletionHandler Handler)
			{
				return PlatformImplementation->SendRequestAsync(PlatformIOObject,
																std::forward<CompletionHandler>(std::move(Handler)));
			}

			template<typename CompletionHandler>
			auto ReadResponseHeadersAsync(implementation_type& PlatformIOObject, CompletionHandler Handler)
			{
				return PlatformImplementation->ReadResponseHeadersAsync(
					PlatformIOObject, std::forward<CompletionHandler>(std::move(Handler)));
			}

			template<typename CompletionHandler>
			auto ReadSomeFromResponseBodyAsync(implementation_type& PlatformIOObject,
											   DynamicBuffer DynamicBufferInstance, CompletionHandler&& Handler)
			{
				return PlatformImplementation->ReadSomeFromResponseBodyAsync(
					PlatformIOObject, DynamicBufferInstance, std::forward<CompletionHandler>(std::move(Handler)));
			}

			template<typename CompletionHandler>
			auto BeginWriteAsync(implementation_type& PlatformIOObject, Modio::FileSize TotalSize,
								 CompletionHandler&& Handler)
			{
				return PlatformImplementation->BeginWriteAsync(PlatformIOObject, TotalSize,
															   std::forward<CompletionHandler>(Handler));
			}

			template<typename CompletionHandler>
			auto WriteSomeAsync(implementation_type& PlatformIOObject, Modio::Detail::Buffer Data,
								CompletionHandler&& Handler)
			{
				return PlatformImplementation->WriteSomeAsync(PlatformIOObject, std::move(Data),
															  std::forward<CompletionHandler>(Handler));
			}

			MODIO_IMPL Modio::Detail::OperationQueue::Ticket GetAPIRequestTicket();

			MODIO_IMPL Modio::Detail::OperationQueue::Ticket GetFileDownloadTicket();

			MODIO_IMPL void Shutdown();

			MODIO_IMPL Modio::ErrorCode ApplyGlobalConfigOverrides(const std::map<std::string, std::string> Overrides)
			{
				auto EnvironmentOverrideUrl = Overrides.find("EnvironmentOverrideUrl");
				if (EnvironmentOverrideUrl != Overrides.end())
				{
					Modio::Detail::SDKSessionData::SetEnvironmentOverrideUrl(EnvironmentOverrideUrl->second);
				}

				return {};
			}

		private:
			MODIO_IMPL void shutdown_service();
		};
	} // namespace Detail

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioHttpService.ipp"
#endif