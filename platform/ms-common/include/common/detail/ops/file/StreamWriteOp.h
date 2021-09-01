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
#include "common/file/FileObjectImplementation.h"
#include "fileapi.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioObjectTrack.h"

#include <asio/yield.hpp>
class StreamWriteOp : public Modio::Detail::BaseOperation<StreamWriteOp>
{
	/// <summary>
	/// Buffer to write to file
	/// </summary>
	Modio::Detail::Buffer Buffer;
	/// <summary>
	/// Reference to the platform-specific implementation of the file object
	/// </summary>
	std::shared_ptr<Modio::Detail::FileObjectImplementation> FileImpl;
	/// <summary>
	/// Offset within the file to write the data
	/// </summary>
	std::uintmax_t FileOffset = 0;
	/// <summary>
	/// Win32 control structure for an async file IO operation
	/// </summary>
	Modio::StableStorage<OVERLAPPED> WriteOpParams;
	/// <summary>
	/// State container for the coroutine invoked in the function call operator overload
	/// </summary>
	asio::coroutine Coroutine;
	/// <summary>
	/// Timer to allow the polling interval to be set for the waiting state of the operation
	/// </summary>
	std::unique_ptr<asio::steady_timer> StatusTimer;

	bool bMovedFrom = false;

public:
	StreamWriteOp(std::shared_ptr<Modio::Detail::FileObjectImplementation> IOObject, Modio::Detail::Buffer Buffer)
		: Buffer(std::move(Buffer)),
		  FileImpl(IOObject),
		  WriteOpParams {}
	{}

	StreamWriteOp(StreamWriteOp&& Other)
		: BaseOperation(std::move(Other)),
		  Buffer(std::move(Other.Buffer)),
		  FileImpl(std::move(Other.FileImpl)),
		  WriteOpParams(std::move(Other.WriteOpParams)),
		  Coroutine(std::move(Other.Coroutine)),
		  StatusTimer(std::move(Other.StatusTimer))
	{
		Other.bMovedFrom = true;
	};

	~StreamWriteOp()
	{
		if (!bMovedFrom)
		{
			if (WriteOpParams && WriteOpParams->hEvent != INVALID_HANDLE_VALUE)
			{
				CloseHandle(WriteOpParams->hEvent);
				WriteOpParams->hEvent = INVALID_HANDLE_VALUE;
			}
		}
	}

	template<typename CoroType>
	void operator()(CoroType& self, std::error_code ec = {})
	{
		if (FileImpl->ShouldCancel())
		{
			self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
			return;
		}

		reenter(Coroutine)
		{
			// Wait for any existing and queued IO operations on this file
			yield FileImpl->BeginExclusiveOperation(std::move(self));

			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
										"Begin write of {} bytes to {}", Buffer.GetSize(),
										FileImpl->GetPath().string());
			WriteOpParams = std::make_shared<OVERLAPPED>();
			WriteOpParams->hEvent = CreateEvent(NULL, false, false, NULL);
			if (!WriteOpParams->hEvent)
			{
				WriteOpParams->hEvent = INVALID_HANDLE_VALUE;
				// Notify the caller that we could not create an event handle
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
											"Could not create event handle");
				self.complete(Modio::make_error_code(Modio::GenericError::CouldNotCreateHandle));
				FileImpl->FinishExclusiveOperation();
				return;
			}

			{
				Modio::FileOffset FilePosition = FileImpl->Tell();
				WriteOpParams->OffsetHigh = FilePosition >> 32;
				WriteOpParams->Offset = (DWORD) FilePosition;
			}

			if (!WriteFile(FileImpl->GetFileHandle(), Buffer.Data(), (DWORD) Buffer.GetSize(), nullptr, WriteOpParams.get()))
			{
				DWORD Error = GetLastError();
				// If Error is IO_PENDING, all is OK. Otherwise, bail
				if (Error != ERROR_IO_PENDING)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Stream Write to file {} failed, error {}",
												FileImpl->GetPath().string(), Error);
					self.complete(std::error_code(static_cast<int>(Error), std::system_category()));
					FileImpl->FinishExclusiveOperation();

					return;
				}
			}
			else
			{
				// File write completed synchronously so no need to wait, complete the operation
				Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Finish write to {}",
											FileImpl->GetPath().string());
				FileImpl->Seek(Modio::FileOffset(Buffer.GetSize()), Modio::Detail::SeekDirection::Forward);
				self.complete(std::error_code {});
				FileImpl->FinishExclusiveOperation();
				return;
			}

			StatusTimer =
				std::make_unique<asio::steady_timer>(Modio::Detail::Services::GetGlobalContext().get_executor());
			while (!HasOverlappedIoCompleted(WriteOpParams.get()))
			{
				StatusTimer->expires_after(std::chrono::milliseconds(1));
				yield StatusTimer->async_wait(std::move(self));
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Finish write to {}",
										FileImpl->GetPath().string());

			FileImpl->Seek(Modio::FileOffset(Buffer.GetSize()), Modio::Detail::SeekDirection::Forward);

			self.complete(std::error_code {});
			FileImpl->FinishExclusiveOperation();
			return;
		}
	}
};

#include <asio/unyield.hpp>