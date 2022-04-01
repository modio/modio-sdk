/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once
#include "ModioGeneratedVariables.h"
#include "http/HttpRequestImplementation.h"
#include "macos/HttpSharedState.h"
#include "macos/detail/ops/http/InitializeHttpOp.h"
#include "macos/detail/ops/http/ReadHttpResponseHeadersOp.h"
#include "macos/detail/ops/http/ReadSomeResponseBodyOp.h"
#include "macos/detail/ops/http/SendHttpRequestOp.h"
#include "macos/detail/ops/http/WriteSomeToRequestOp.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/http/IHttpServiceImplementation.h"
#include "modio/http/ModioHttpParams.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace Modio
{
	namespace Detail
	{
		class HttpImplementation : public Modio::Detail::IHttpServiceImplementation
		{
			std::shared_ptr<HttpSharedState> HttpState;
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

			HttpImplementation(asio::io_context::service& OwningService) : OwningService(OwningService) {}

			virtual ~HttpImplementation() {}

			template<typename CompletionToken>
			auto InitializeHTTPAsync(CompletionToken&& Token)
			{
				constexpr const char* ModioAgentString =
					"Modio SDK v2 built from " MODIO_COMMIT_HASH ":" MODIO_TARGET_PLATFORM_ID;
				HttpState = std::make_shared<HttpSharedState>();
				HttpState->UserAgentString = ModioAgentString;
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					InitializeHttpOp(ModioAgentString, HttpState), Token,
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
				Modio::Detail::Buffer EmptyBuffer(TotalLength);
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					WriteSomeToRequestOp(PlatformIOObjectInstance, std::move(EmptyBuffer), HttpState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
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
