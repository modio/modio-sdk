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

#include "http/HttpRequestImplementation.h"
#include "ios/AppleHttpRequest.h"
#include "ios/AppleHttpSession.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include <cstring>
#include <memory>
#include <string>

namespace Modio
{
	namespace Detail
	{
		struct HttpSharedState
		{
			std::unique_ptr<Modio::Detail::Apple::HttpSession> Session;
			std::string UserAgentString;

			HttpSharedState() : Session(std::make_unique<Modio::Detail::Apple::HttpSession>()) {}

			Modio::ErrorCode Initialize()
			{
				return Session->Initialize(UserAgentString);
			}

			void InitializeRequest(std::shared_ptr<HttpRequestImplementation> Request, Modio::ErrorCode& ec)
			{
				Request->AppleRequest = Session->CreateRequest();

				const std::string FullURL =
					"https://" + Request->Parameters.GetServerAddress() +
					Request->Parameters.GetFormattedResourcePath();
				Request->AppleRequest->SetURL(FullURL);
				Request->AppleRequest->SetVerb(Request->Parameters.GetVerb());

				for (const Modio::Detail::HttpRequestParams::Header& Hdr : Request->Parameters.GetHeaders())
				{
					Request->AppleRequest->AddHeader(Hdr.first, Hdr.second);
				}

				const Verb V = Request->Parameters.GetTypedVerb();
				const bool HasBody = (V == Verb::POST || V == Verb::PUT || V == Verb::DELETE);
				if (HasBody)
				{
					Modio::Optional<std::string> EncodedPayload =
						Request->Parameters.GetUrlEncodedPayload();
					if (EncodedPayload.has_value())
					{
						// URL-encoded form / JSON: the whole body is known up-front.
						const std::string& Payload = EncodedPayload.value();
						Modio::Detail::Buffer Body(Payload.size());
						std::memcpy(Body.Data(), Payload.data(), Payload.size());
						Request->AppleRequest->SetBody(std::move(Body));
					}
					else
					{
						// Streaming body (e.g. multipart upload). WriteSomeToRequestOp
						// will push the bytes after Start fires.
						Request->AppleRequest->BeginStreamedBody(Request->Parameters.GetPayloadSize());
					}
				}

				Modio::Detail::Logger().Log(
					Modio::LogLevel::Info, Modio::LogCategory::Http,
					"Initializing {0} request for {1} {2}",
					Request->Parameters.GetVerb(),
					Request->Parameters.GetServerAddress(),
					Request->Parameters.GetFormattedResourcePath());

				(void) ec;
			}

			void Close()
			{
				Session->Close();
			}

			bool IsClosing() const
			{
				return Session->IsClosing();
			}
		};
	} // namespace Detail
} // namespace Modio
