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

#include <asio/yield.hpp>

class ReadSomeFromFileOp
{
	Modio::Detail::Buffer Buffer;
	std::shared_ptr<Modio::Detail::FileObjectImplementation> FileImpl;
	std::uintmax_t Offset;
	std::uintmax_t Length;
	Modio::StableStorage<OVERLAPPED> ReadOpParams;
	asio::coroutine Coroutine;
	std::unique_ptr<asio::steady_timer> StatusTimer;
	Modio::StableStorage<DWORD> NumberOfBytesRead;

public:
	ReadSomeFromFileOp(std::shared_ptr<Modio::Detail::FileObjectImplementation> IOObject, std::uintmax_t Offset,
					 std::uintmax_t Length)
		: Buffer(Length, 4 * 1024),
		  FileImpl(IOObject),
		  Offset(Offset),
		  Length(Length),
		  ReadOpParams(std::make_shared<OVERLAPPED>()),
		  NumberOfBytesRead(std::make_shared<DWORD>(0))

	{}

	ReadSomeFromFileOp(ReadSomeFromFileOp&& Other)
		: Buffer(std::move(Other.Buffer)),
		  FileImpl(Other.FileImpl),
		  Offset(Other.Offset),
		  Length(Other.Length),
		  ReadOpParams(std::move(Other.ReadOpParams)),
		  Coroutine(std::move(Other.Coroutine)),
		  StatusTimer(std::move(Other.StatusTimer)),
		  NumberOfBytesRead(std::move(Other.NumberOfBytesRead))
	{}

	~ReadSomeFromFileOp()
	{
		DWORD HandleInfo;
		if (ReadOpParams && GetHandleInformation(ReadOpParams->hEvent, &HandleInfo))
		{
			CloseHandle(ReadOpParams->hEvent);
		}
	}

	template<typename CoroType>
	void operator()(CoroType& self, std::error_code ec = {})
	{
		if (FileImpl->ShouldCancel())
		{
			self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled), {});
			return;
		}

		reenter(Coroutine)
		{
			// Wait for any other operations on the file to complete
			// Possibly, we can make the release process for this 'lock' automatic
			yield FileImpl->BeginExclusiveOperation(std::move(self));

			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
								"Begin read of {} bytes from {} at {}", Length, FileImpl->GetPath().string(), Offset);

			ReadOpParams->hEvent = CreateEvent(NULL, false, false, NULL);

			if (!ReadOpParams->hEvent)
			{
				self.complete(Modio::make_error_code(Modio::GenericError::CouldNotCreateHandle), {});
				FileImpl->FinishExclusiveOperation();

				return;
			}

			ReadOpParams->OffsetHigh = Offset >> 32;
			ReadOpParams->Offset = (DWORD) Offset;

			if (!ReadFile(FileImpl->GetFileHandle(), Buffer.Data(), Length, NumberOfBytesRead.get(),
						  ReadOpParams.get()))
			{
				DWORD Error = GetLastError();
				if (Error != ERROR_IO_PENDING)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
										"Read from {} failed, error = {}", FileImpl->GetPath().string(), Error);
					self.complete(std::error_code(static_cast<int>(Error), std::system_category()), {});
					FileImpl->FinishExclusiveOperation();

					return;
				}
			}
			else
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Finish read from {}",
									FileImpl->GetPath().string());
				*NumberOfBytesRead = ReadOpParams->InternalHigh;
				if (*NumberOfBytesRead < Length)
				{
					Modio::Detail::Buffer(*NumberOfBytesRead, 4 * 1024);
					self.complete(std::error_code {},
								  Buffer.CopyRange(Buffer.begin(), Buffer.begin() + *NumberOfBytesRead));
				}
				else
				{
					self.complete(std::error_code {}, std::move(Buffer));
				}
				FileImpl->FinishExclusiveOperation();

				return;
			}

			StatusTimer =
				std::make_unique<asio::steady_timer>(Modio::Detail::Services::GetGlobalContext().get_executor());
			while (!HasOverlappedIoCompleted(ReadOpParams.get()))
			{
				StatusTimer->expires_after(std::chrono::milliseconds(1));
				yield StatusTimer->async_wait(std::move(self));
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Finish read from {}",
								FileImpl->GetPath().string());
			*NumberOfBytesRead = ReadOpParams->InternalHigh;
			if (*NumberOfBytesRead < Length)
			{
				Modio::Detail::Buffer(*NumberOfBytesRead, 4 * 1024);
				self.complete(std::error_code {},
							  Buffer.CopyRange(Buffer.begin(), Buffer.begin() + *NumberOfBytesRead));
			}
			else
			{
				self.complete(std::error_code {}, std::move(Buffer));
			}
			FileImpl->FinishExclusiveOperation();

			return;
		}
	}
};

#include <asio/unyield.hpp>