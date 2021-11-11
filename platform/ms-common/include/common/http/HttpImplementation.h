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
#include "common/detail/ops/http/ReadHttpResponseHeadersOp.h"
#include "common/detail/ops/http/ReadSomeResponseBodyOp.h"
#include "common/detail/ops/http/SendHttpRequestOp.h"
#include "common/detail/ops/http/WriteSomeToRequestOp.h"
#include "common/http/HttpRequestImplementation.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/http/IHttpServiceImplementation.h"
#include "modio/http/ModioHttpParams.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <winhttp.h>

namespace Modio
{
	namespace Detail
	{
		/// @brief Common base class for HTTP Implementation on MS platforms - consider this an implementation detail of
		/// MS platforms. Other platforms should implement @xref:Modio::Detail::IHttpServiceImplementation[] directly.
		/// Initialization of the HTTP service is the only current point of variance between MS platforms and so
		/// async_InitializeHttp is the only method that delegates onto the subplatform for creation of the correct
		/// operation. If other sub platforms require customization of SendHttpRequest, ReadResponseHeaders, or
		/// ReadSomeResponseBody, then the creation of those operations should be delegated to subplatforms in the same
		/// fashion.
		/// @tparam Subplatform CRTP Parameter for the specific subplatform being implemented
		/// @tparam SharedStateType Subplatform-specific shared state type
		template<typename Subplatform, typename SharedStateType>
		class HttpImplementationBase : public Modio::Detail::IHttpServiceImplementation
		{
			std::shared_ptr<SharedStateType> HttpState;
			asio::io_context::service& OwningService;

		public:
			using IOObjectImplementationType = std::shared_ptr<HttpRequestImplementation>;

			void InitializeIOObjectImplementation(IOObjectImplementationType& IOObjectImpl)
			{
				IOObjectImpl.reset(new HttpRequestImplementation());
			}

			void MoveIOObjectImplementation(IOObjectImplementationType& Implementation,
											IOObjectImplementationType& OtherImplementation)
			{
				Implementation = std::move(OtherImplementation);
			}

			HttpImplementationBase(asio::io_context::service& OwningService) : OwningService(OwningService) {}
			virtual ~HttpImplementationBase() {}

			template<typename CompletionToken>
			auto InitializeHTTPAsync(CompletionToken&& Token)
			{
				constexpr const wchar_t* ModioAgentString =
					L"Modio SDK v2 built from " MODIO_COMMIT_HASH ":" MODIO_TARGET_PLATFORM_ID;
				HttpState = std::make_shared<SharedStateType>(nullptr);
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					static_cast<Subplatform*>(this)->MakeInitializeHttpOp(ModioAgentString, HttpState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionToken>
			auto SendRequestAsync(IOObjectImplementationType PlatformIOObjectInstance, CompletionToken&& Token)
			{
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					SendHttpRequestOp(PlatformIOObjectInstance, HttpState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionToken>
			auto ReadResponseHeadersAsync(IOObjectImplementationType PlatformIOObjectInstance, CompletionToken Token)
			{
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					ReadHttpResponseHeadersOp(PlatformIOObjectInstance, HttpState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionToken>
			auto ReadSomeFromResponseBodyAsync(IOObjectImplementationType PlatformIOObjectInstance,
											   Modio::Detail::DynamicBuffer DynamicBufferInstance,
											   CompletionToken Token)
			{
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					ReadSomeResponseBodyOp(PlatformIOObjectInstance, DynamicBufferInstance, HttpState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionToken>
			auto BeginWriteAsync(IOObjectImplementationType PlatformIOObjectInstance, Modio::FileSize TotalLength,
								 CompletionToken&& Token)
			{
				/*return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					BeginHttpRequestForWriteOp(PlatformIOObjectInstance, HttpState, TotalLength), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());*/
			}

			template<typename CompletionToken>
			auto WriteSomeAsync(IOObjectImplementationType PlatformIOObjectInstance, Modio::Detail::Buffer Data,
								CompletionToken&& Token)
			{
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					WriteSomeToRequestOp(PlatformIOObjectInstance, std::move(Data), HttpState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			void Shutdown()
			{
				HttpState->Close();
			}
		};
	} // namespace Detail
} // namespace Modio
