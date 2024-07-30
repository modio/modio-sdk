/*
 *  Copyright (C) 2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/ModioConstants.h"
#include "modio/detail/serialization/ModioUploadPartSerialization.h"
#include "modio/http/ModioHttpParams.h"

MODIO_DIAGNOSTIC_PUSH

MODIO_ALLOW_DEPRECATED_SYMBOLS

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class MultipartGetUploadedOp
		{
			Modio::Detail::HttpRequestParams GetPartsRequest;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			std::shared_ptr<Modio::Detail::UploadSessionPartList> ResponseParts;
			asio::coroutine Coroutine;

		public:
			MultipartGetUploadedOp(std::shared_ptr<Modio::Detail::UploadSessionPartList> Response,
								   Modio::ModID CurrentModID, Modio::Detail::UploadSession USession)
			{
				ResponseParts = Response;
				GetPartsRequest = Modio::Detail::GetMultipartUploadPartsRequest
									  .SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
									  .SetModID(CurrentModID)
									  .AddQueryParamRaw("upload_id", USession.UploadID.value());
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(Coroutine)
				{
					// Request all available "Upload Sessions"
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBuffer, GetPartsRequest, Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
													"Error retrieving multipart non-uploaded parts");
						Self.complete(ec);
						return;
					}

					{
						Modio::Optional<Modio::Detail::UploadSessionPartList> OptSession =
							TryMarshalResponse<Modio::Detail::UploadSessionPartList>(ResponseBuffer);
						if (OptSession.has_value() == false)
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}

						*ResponseParts = OptSession.value();
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"Parts uploaded identified: {}", ResponseParts->Size());
					Self.complete({});
				}
			}
		};
#include <asio/unyield.hpp>

		template<typename CompletionTokenType>
		auto MultipartGetUploadedAsync(std::shared_ptr<Modio::Detail::UploadSessionPartList> Response,
									   Modio::ModID CurrentModID, Modio::Detail::UploadSession USession,
									   CompletionTokenType&& Token)
		{
			return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				MultipartGetUploadedOp(std::move(Response), CurrentModID, USession), Token,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

MODIO_DIAGNOSTIC_POP
