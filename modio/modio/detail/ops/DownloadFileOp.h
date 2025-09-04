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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioOperationQueue.h"
#include "modio/file/ModioFile.h"
#include "modio/http/ModioHttpRequest.h"
#include <asio/yield.hpp>

MODIO_DIAGNOSTIC_PUSH

MODIO_ALLOW_DEPRECATED_SYMBOLS


namespace Modio
{
	namespace Detail
	{
		/// @brief Operation which downloads an arbitrary file to an arbitrary filesystem path
		class DownloadFileOp : public Modio::Detail::BaseOperation<DownloadFileOp>
		{
			Modio::StableStorage<Modio::Detail::HttpRequest> Request {};
			asio::coroutine Coroutine {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			Modio::StableStorage<Modio::Detail::File> File {};

			Modio::StableStorage<std::uintmax_t> CurrentFilePosition {};
			Modio::StableStorage<bool> EndOfFileReached {};
			Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo {};

			Modio::Optional<std::uint64_t> ExpectedFilesize {};

			struct DownloadFileImpl
			{
				Modio::Detail::OperationQueue::Ticket DownloadTicket;
				std::uint8_t RedirectLimit = 8;
				bool bRequiresRedirect = false;

			public:
				DownloadFileImpl(Modio::Detail::OperationQueue::Ticket DownloadTicket)
					: DownloadTicket(std::move(DownloadTicket)) {}
			};

			Modio::StableStorage<DownloadFileImpl> Impl {};

		public:
			DownloadFileOp(const Modio::Detail::HttpRequestParams RequestParams,
						   Modio::filesystem::path DestinationPath,
						   Modio::Detail::OperationQueue::Ticket DownloadTicket,
						   Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo,
							Modio::Optional<std::uint64_t> Filesize)
				: ProgressInfo(ProgressInfo)
			{
				ExpectedFilesize = Filesize;
				File = std::make_shared<Modio::Detail::File>(DestinationPath += Modio::filesystem::path(".download"),
															 Modio::Detail::FileMode::ReadWrite, false);
				// Initialize CurrentFilePosition to 0 - we'll set the actual value after truncate
				CurrentFilePosition = std::make_shared<std::uintmax_t>(0ULL);
				// Initialize the request without range header - we'll update it after setting the position
				Request = std::make_shared<Modio::Detail::HttpRequest>(RequestParams);
				EndOfFileReached = std::make_shared<bool>(false);
				Impl = std::make_shared<DownloadFileImpl>(std::move(DownloadTicket));
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(DownloadFile);

				if (Impl->DownloadTicket.WasCancelled())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				// May need to manually check bDownloadingMod and lock progressinfo if this returns true when we never
				// had a value in the pointer
				if (ProgressInfo.has_value() && ProgressInfo->expired())
				{
					// If we abort a download, truncate the file to zero bytes so that we start from the beginning at
					// next download (can't delete it here as it's locked)
					if (File)
					{
						ec = File->Truncate(Modio::FileOffset(0));

						if (ec)
						{
							Self.complete(ec);
							return;
						}
						// Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFile(File->GetPath());
					}
					Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
					return;
				}

				// Temporary optional to hold buffers without causing issues with coroutine switch statement
				Modio::Optional<Modio::Detail::Buffer> CurrentBuffer;

				reenter(Coroutine)
				{
					// Initialize file position and perform truncate/seek operations
					*CurrentFilePosition = File->GetFileSize() - (File->GetFileSize() % (static_cast<std::int64_t>(1024) * 1024));

					ec = File->Truncate(Modio::FileOffset(*CurrentFilePosition));
					if (ec)
					{
						Self.complete(ec);
						return;
					}

					File->Seek(Modio::FileOffset(*CurrentFilePosition));

					// Update request with range header now that we have the correct position
					Request->Parameters().SetRange(Modio::FileOffset(*CurrentFilePosition), {});

					yield Impl->DownloadTicket.WaitForTurnAsync(std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
												"Beginning download of file {}", Modio::ToModioString(File->GetPath().u8string()));
					do
					{
						yield Request->SendAsync(std::move(Self));

						if (ec)
						{
							Self.complete(ec);
							return;
						}

						yield Request->ReadResponseHeadersAsync(std::move(Self));

						if (ec)
						{
							Self.complete(ec);
							return;
						}

						if (Modio::Optional<std::uint32_t> RetryAfter = Request->GetRetryAfter())
						{
							Modio::Detail::SDKSessionData::MarkAsRateLimited(std::int32_t(RetryAfter.value()));
						}

						if (Request->GetResponseCode() >= 301 && Request->GetResponseCode() < 400)
						{
							Impl->bRequiresRedirect = true;
							if (Impl->RedirectLimit)
							{
								Impl->RedirectLimit--;
							}
							else
							{
								Self.complete(Modio::make_error_code(Modio::HttpError::ExcessiveRedirects));
								return;
							}
							// Ask the existing request for the redirect URL from the headers
							if (Modio::Optional<std::string> RedirectedURL = Request->GetRedirectURL())
							{
								// Change the URL on the existing parameters provided it matches our whitelist
								Modio::Optional<Modio::Detail::HttpRequestParams> RedirectedParams =
									Modio::Detail::HttpRequestParams::FileDownload(RedirectedURL.value(),
																				   Request->Parameters());
								// URL did not match the whitelist
								if (!RedirectedParams)
								{
									Modio::Detail::Logger().Log(
										Modio::LogLevel::Error, Modio::LogCategory::Http,
										"download of file {} redirected to URL outside whitelist to: {}",
										Modio::ToModioString(File->GetPath().u8string()), RedirectedURL.value());
									Self.complete(Modio::make_error_code(Modio::HttpError::ResourceNotAvailable));
									return;
								}
								// Swap out the request for a new one for the redirect
								Request = std::make_shared<Modio::Detail::HttpRequest>(RedirectedParams.value());
							}
							else
							{
								// Got a 302 but we dont have a new location to go to
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
															"302 but no redirect target");
								Self.complete(Modio::make_error_code(Modio::HttpError::ResourceNotAvailable));
								return;
							}
						}
						else if (Request->GetResponseCode() != 200 && Request->GetResponseCode() != 206)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"download of file {} got response {}",
														Modio::ToModioString(File->GetPath().u8string()), Request->GetResponseCode());
							Self.complete(Modio::make_error_code(Modio::HttpError::ResourceNotAvailable));
							return;
						}
						else
						{
							Impl->bRequiresRedirect = false;
						}

					} while (Impl->bRequiresRedirect && Impl->RedirectLimit);

					while (!ec)
					{
						// Read in a chunk from the response
						yield Request->ReadSomeFromResponseBodyAsync(ResponseBodyBuffer, std::move(Self));
						if (ec && ec != Modio::make_error_code(Modio::GenericError::EndOfFile))
						{
							Self.complete(ec);
							return;
						}
						else if (ec == Modio::make_error_code(Modio::GenericError::EndOfFile))
						{
							// Cache the EOF state, because ec gets mutated by the calls to async_WriteSomeAt below
							*EndOfFileReached = true;
						}

						// Some implementations of ReadSomeFromResponseBodyAsync may store multiple buffers in a single
						// call so make sure we steal all of them
						while ((CurrentBuffer = ResponseBodyBuffer.TakeInternalBuffer()))
						{
							*CurrentFilePosition += CurrentBuffer->GetSize();

							if (ProgressInfo.has_value())
							{
								if (!ProgressInfo->expired())
								{
									auto Progress = ProgressInfo->lock();
									SetCurrentProgress(*Progress.get(), Modio::FileSize(*CurrentFilePosition));
								}
								else
								{
									Self.complete(
										Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
									return;
								}
							}

							yield File->WriteSomeAtAsync(*CurrentFilePosition - CurrentBuffer->GetSize(),
														 std::move(CurrentBuffer.value()), std::move(Self));
						}

						// Did we receive the entire response body?
						if (*EndOfFileReached)
						{
							Modio::filesystem::path Destination = File->GetPath().replace_extension();
							if (Modio::ErrorCode RenameResult = File->Rename(Destination))
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
															"Could not rename downloaded file to {}",
															Modio::ToModioString(Destination.u8string()));
								File.reset();

								Self.complete(RenameResult);
								return;
							}
							else
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
															"Download of {} completed with size: {}; expected filesize was {}",
															Modio::ToModioString(File->GetPath().u8string()), File->GetFileSize(), ExpectedFilesize.has_value() ? ExpectedFilesize.value() : 0);

								if (ExpectedFilesize.has_value() && (File->GetFileSize() != ExpectedFilesize))
								{
									Modio::Detail::Logger().Log(
										Modio::LogLevel::Error, Modio::LogCategory::Http,
										"Downloaded file was not the expected size; returning");

									Self.complete(Modio::make_error_code(FilesystemError::WriteError));
									File.reset();
									return;
								}

								// Clean up
								File.reset();
								Self.complete(Modio::ErrorCode {});
								return;
							}
						}
					}

					if (ec && ec != Modio::make_error_code(Modio::GenericError::EndOfFile))
					{
						Self.complete(ec);
						return;
					}
					else
					{
						File.reset();
						Self.complete(Modio::ErrorCode {});
						return;
					}
				}
			}
		};

		template<typename CompletionTokenType>
		auto DownloadFileAsync(Modio::Detail::HttpRequestParams DownloadParameters,
							   Modio::filesystem::path DestinationPath,
							   Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ModProgress,
							   Modio::Optional<std::uint64_t> Filesize,
							   CompletionTokenType&& Token)
		{
			return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				DownloadFileOp(
					DownloadParameters, DestinationPath,
					Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetFileDownloadTicket(),
					ModProgress, Filesize),
				Token, Modio::Detail::Services::GetGlobalContext().get_executor());
		}

		// Downloads file as API request, bypassing the download queue.  For use with files such as images that
		// shouldn't have to wait for mod downloads to complete.
		template<typename CompletionTokenType>
		auto DownloadFileAsApiRequestAsync(Modio::Detail::HttpRequestParams DownloadParameters,
										   Modio::filesystem::path DestinationPath, Modio::Optional<std::uint64_t> Filesize, CompletionTokenType&& Token)
		{
			return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				DownloadFileOp(
					DownloadParameters, DestinationPath,
					Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetAPIRequestTicket(), {}, Filesize),
				Token, Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>


MODIO_DIAGNOSTIC_POP
