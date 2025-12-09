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

MODIO_DIAGNOSTIC_PUSH

MODIO_ALLOW_DEPRECATED_SYMBOLS


#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class MultipartGetSessionOp
		{
			Modio::Detail::HttpRequestParams GetSessionsRequest {};
			Modio::Detail::DynamicBuffer ResponseBuffer {};
			std::shared_ptr<Modio::Detail::UploadSession> UploadSession {};
			Modio::ModID ModID {};
			ModioAsio::coroutine Coroutine {};

		public:
			MultipartGetSessionOp(std::shared_ptr<Modio::Detail::UploadSession> ResponseSession,
								  Modio::ModID CurrentModID)
				: ModID(CurrentModID)
			{
				UploadSession = ResponseSession;
				GetSessionsRequest = Modio::Detail::GetMultipartUploadSessionsRequest
										 .SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
										 .SetModID(CurrentModID);
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(Coroutine)
				{
					// Request all available "Upload Sessions"
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBuffer, GetSessionsRequest, Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
													"Error retrieving all multipart open sessions");
						Self.complete(ec);
						return;
					}

					// Make sure that the UploadID is empty before trying to marshall the response
					UploadSession->UploadID = {};

					{
						// After the first request, we expect a "Upload Session", which is used
						// to append all other "chunks" of the large file.
						Modio::Optional<Modio::Detail::UploadSessionList> OptSession =
							TryMarshalResponse<Modio::Detail::UploadSessionList>(ResponseBuffer);
						if (OptSession.has_value() == false)
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}

						// Check each session in the list for one that needs to complete
						for (Modio::Detail::UploadSession UpSession : OptSession.value())
						{
							if (UpSession.UploadStatus == UploadSession::Status::Incomplete)
							{
								UploadSession->UploadID = UpSession.UploadID;
								UploadSession->UploadStatus = UpSession.UploadStatus;
								// With one found, return immediatelly
								break;
							}
						}
					}

					// We did not find any open UploadIDs. Return error
					if (!UploadSession->UploadID.has_value() || UploadSession->UploadID.value().empty())
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
													"There are no incomplete upload session");
						Self.complete(Modio::make_error_code(Modio::ModManagementError::NoPendingWork));
						return;
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"Unfinished upload session {} identified",
												UploadSession->UploadID.value());
					Self.complete({});
				}
			}
		};
#include <asio/unyield.hpp>

		template<typename CompletionTokenType>
		auto MultipartGetSessionAsync(std::shared_ptr<Modio::Detail::UploadSession> Response, Modio::ModID CurrentModID,
									  CompletionTokenType&& Token)
		{
			return ModioAsio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				MultipartGetSessionOp(std::move(Response), CurrentModID), Token,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

MODIO_DIAGNOSTIC_POP
