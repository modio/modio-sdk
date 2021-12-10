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
#include "http/HttpRequestImplementation.h"
#include "linux/HttpSharedState.h"
#include "linux/detail/ops/http/InitializeHttpOp.h"
#include "linux/detail/ops/http/ReadHttpResponseHeadersOp.h"
#include "linux/detail/ops/http/ReadSomeResponseBodyOp.h"
#include "linux/detail/ops/http/SendHttpRequestOp.h"
#include "linux/detail/ops/http/SSLConnectionWriteOp.h"
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
				Modio::Detail::DynamicBuffer EmptyBuffer;
				EmptyBuffer.grow(TotalLength);
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					SSLConnectionWriteOp(PlatformIOObjectInstance, std::move(EmptyBuffer), HttpState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionToken>
			auto WriteSomeAsync(IOObjectImplementationType PlatformIOObjectInstance, Modio::Detail::Buffer Data,
								CompletionToken&& Token)
			{
				Modio::Detail::DynamicBuffer DataDBuffer;
				DataDBuffer.AppendBuffer(std::move(Data));
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					SSLConnectionWriteOp(PlatformIOObjectInstance, std::move(DataDBuffer), HttpState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			void Shutdown()
			{
				HttpState->Close();
			}
		};
	} // namespace Detail
} // namespace Modio
