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
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioOperationQueue.h"
#include "modio/file/ModioFile.h"
#include "modio/http/ModioHttpRequest.h"
#include <asio/yield.hpp>

/// @brief Operation which downloads an arbitrary file to an arbitrary filesystem path
class DownloadFileOp : public Modio::Detail::BaseOperation<DownloadFileOp>
{
	Modio::StableStorage<Modio::Detail::HttpRequest> Request;
	asio::coroutine Coroutine;
	Modio::Detail::DynamicBuffer ResponseBodyBuffer;
	Modio::StableStorage<Modio::Detail::File> File;

	Modio::StableStorage<std::uintmax_t> CurrentFilePosition;
	Modio::StableStorage<bool> EndOfFileReached;
	Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo;
	bool bDownloadingMod = false;

	struct DownloadFileImpl
	{
		Modio::Detail::OperationQueue::Ticket DownloadTicket;
		std::uint8_t RedirectLimit = 8;
		bool bRequiresRedirect = false;

	public:
		DownloadFileImpl(Modio::Detail::OperationQueue::Ticket DownloadTicket)
			: DownloadTicket(std::move(DownloadTicket)) {};
	};

	Modio::StableStorage<DownloadFileImpl> Impl;

public:
	DownloadFileOp(const Modio::Detail::HttpRequestParams RequestParams, Modio::filesystem::path DestinationPath,
				   Modio::Detail::OperationQueue::Ticket DownloadTicket,
				   Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo)
		: ProgressInfo(ProgressInfo)
	{
		File = std::make_shared<Modio::Detail::File>(DestinationPath += Modio::filesystem::path(".download"), false);
		CurrentFilePosition =
			std::make_shared<std::uintmax_t>(File->GetFileSize() - (File->GetFileSize() % (1024 * 1024)));
		File->Truncate(Modio::FileOffset(*CurrentFilePosition));
		File->Seek(Modio::FileOffset(*CurrentFilePosition));
		Request = std::make_shared<Modio::Detail::HttpRequest>(
			RequestParams.SetRange(Modio::FileOffset(*CurrentFilePosition), {}));
		EndOfFileReached = std::make_shared<bool>(false);
		Impl = std::make_shared<DownloadFileImpl>(std::move(DownloadTicket));
	};

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		// May need to manually check bDownloadingMod and lock progressinfo if this returns true when we never had a
		// value in the pointer
		if (ProgressInfo.has_value() && ProgressInfo->expired())
		{
			// If we abort a download, truncate the file to zero bytes so that we start from the beginning at next
			// download (can't delete it here as it's locked)
			if (File)
			{
				File->Truncate(Modio::FileOffset(0));
				// Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFile(File->GetPath());
			}
			Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
			return;
		}

		// Temporary optional to hold buffers without causing issues with coroutine switch statement
		Modio::Optional<Modio::Detail::Buffer> CurrentBuffer;

		reenter(Coroutine)
		{
			yield Impl->DownloadTicket.WaitForTurnAsync(std::move(Self));

			if (ec)
			{
				Self.complete(ec);
				return;
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
										"Beginning download of file {}", File->GetPath().u8string());
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

				
				if (Request->GetResponseCode() == 302)
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
							Request->Parameters().RedirectURL(RedirectedURL.value());
						// URL did not match the whitelist
						if (!RedirectedParams)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
														"download of file {} redirected to URL outside whitelist",
														File->GetPath().u8string());
							Self.complete(Modio::make_error_code(Modio::HttpError::ResourceNotAvailable));
							return;
						}
						// Swap out the request for a new one for the redirect
						Request = std::make_shared<Modio::Detail::HttpRequest>(RedirectedParams.value());
					}
					else
					{
						// Got a 302 but we dont have a new location to go to
						Self.complete(Modio::make_error_code(Modio::HttpError::ResourceNotAvailable));
						return;
					}
				}
				else if (Request->GetResponseCode() != 200 && Request->GetResponseCode() != 206)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"download of file {} got response {}", File->GetPath().u8string(), Request->GetResponseCode());
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
				
				// Some implementations of ReadSomeFromResponseBodyAsync may store multiple buffers in a single call
				// so make sure we steal all of them
				while ((CurrentBuffer = ResponseBodyBuffer.TakeInternalBuffer()))
				{
					*CurrentFilePosition += CurrentBuffer->GetSize();

					if (ProgressInfo.has_value())
					{
						if (!ProgressInfo->expired())
						{
							ProgressInfo->lock()->CurrentlyDownloadedBytes = Modio::FileSize(*CurrentFilePosition);
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
							return;
						}
					}

					yield File->WriteSomeAtAsync(*CurrentFilePosition - CurrentBuffer->GetSize(),
												 std::move(CurrentBuffer.value()), std::move(Self));
				}

				if (*EndOfFileReached)
				{
					Modio::filesystem::path Destination = File->GetPath().replace_extension();
					if (Modio::ErrorCode RenameResult = File->Rename(Destination))
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
													"Could not rename downloaded file to {}", Destination.u8string());
						File.reset();

						Self.complete(RenameResult);
						return;
					}
					else
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
													"Download of {} completed with size: {}", File->GetPath().u8string(), File->GetFileSize());
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
#include <asio/unyield.hpp>