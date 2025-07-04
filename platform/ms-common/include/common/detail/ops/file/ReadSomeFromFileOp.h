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
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/timer/ModioTimer.h"

#include <asio/yield.hpp>

class ReadSomeFromFileOp
{
	Modio::Detail::Buffer Buffer;
	std::shared_ptr<Modio::Detail::FileObjectImplementation> FileImpl {};
	std::uintmax_t Offset = 0;
	std::uintmax_t Length = 0;
	Modio::StableStorage<OVERLAPPED> ReadOpParams {};
	asio::coroutine Coroutine {};
	Modio::Detail::Timer StatusTimer {};
	Modio::StableStorage<DWORD> NumberOfBytesRead {};

public:
	ReadSomeFromFileOp(std::shared_ptr<Modio::Detail::FileObjectImplementation> IOObject, std::uintmax_t Offset,
					   std::uintmax_t Length)
		: Buffer(Length, 4 * 1024),
		  FileImpl(IOObject),
		  Offset(Offset),
		  Length(Length),
		  ReadOpParams(std::make_shared<OVERLAPPED>()),
		  NumberOfBytesRead(std::make_shared<DWORD>(0U))

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
	void operator()(CoroType& Self, std::error_code MODIO_UNUSED_ARGUMENT(ec) = {})
	{
		if (FileImpl->ShouldCancel())
		{
			Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled), {});
			return;
		}
		MODIO_PROFILE_SCOPE(ReadSomeFromFile);

		reenter(Coroutine)
		{
			if (Length == 0)
			{
				Self.complete({}, Modio::Detail::Buffer(0));
				return;
			}

			yield asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));

			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
										"Begin read of {} bytes from {} at {}", Length, FileImpl->GetPath().string(),
										Offset);

			ReadOpParams->hEvent = CreateEvent(nullptr, false, false, nullptr);

			if (!ReadOpParams->hEvent)
			{
				Self.complete(Modio::make_error_code(Modio::GenericError::CouldNotCreateHandle), {});
				
				return;
			}

			ReadOpParams->OffsetHigh = Offset >> 32;
			ReadOpParams->Offset = DWORD(Offset);

			if (!ReadFile(FileImpl->GetFileHandle(), Buffer.Data(), Length, NumberOfBytesRead.get(),
						  ReadOpParams.get()))
			{
				DWORD Error = GetLastError();
				if (Error != ERROR_IO_PENDING)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"ReadSomeFromFileOp ReadFile {} failed, error code = {}",
												FileImpl->GetPath().string(), Error);
					Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError), {});
					
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
					Self.complete(std::error_code {},
								  Buffer.CopyRange(Buffer.begin(), Buffer.begin() + *NumberOfBytesRead));
				}
				else
				{
					Self.complete(std::error_code {}, std::move(Buffer));
				}
				
				return;
			}

			while (!HasOverlappedIoCompleted(ReadOpParams.get()))
			{
				StatusTimer.ExpiresAfter(Modio::Detail::Constants::Configuration::PollInterval);
				yield StatusTimer.WaitAsync(std::move(Self));
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Finish read from {}",
										FileImpl->GetPath().string());
			*NumberOfBytesRead = ReadOpParams->InternalHigh;
			if (*NumberOfBytesRead < Length)
			{
				Modio::Detail::Buffer(*NumberOfBytesRead, 4 * 1024);
				Self.complete(std::error_code {},
							  Buffer.CopyRange(Buffer.begin(), Buffer.begin() + *NumberOfBytesRead));
			}
			else
			{
				Self.complete(std::error_code {}, std::move(Buffer));
			}
			
			return;
		}
	}
};

#include <asio/unyield.hpp>