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
#include "modio/timer/ModioTimer.h"

#include <asio/yield.hpp>
class StreamReadOp
{
	Modio::Detail::Buffer Buffer;
	std::shared_ptr<Modio::Detail::FileObjectImplementation> FileImpl;
	std::uintmax_t Length;
	Modio::StableStorage<OVERLAPPED> ReadOpParams;
	asio::coroutine Coroutine;
	Modio::Detail::Timer StatusTimer;
	Modio::StableStorage<DWORD> NumberOfBytesRead;
	Modio::Detail::DynamicBuffer Destination;

public:
	StreamReadOp(std::shared_ptr<Modio::Detail::FileObjectImplementation> IOObject, std::uintmax_t MaxBytesToRead,
				 Modio::Detail::DynamicBuffer Destination)
		: Buffer(MaxBytesToRead, 4 * 1024),
		  FileImpl(IOObject),
		  Length(MaxBytesToRead),
		  ReadOpParams(std::make_shared<OVERLAPPED>()),
		  NumberOfBytesRead(std::make_shared<DWORD>(0U)),
		  Destination(Destination)

	{}

	StreamReadOp(StreamReadOp&& Other)
		: Buffer(std::move(Other.Buffer)),
		  FileImpl(Other.FileImpl),
		  Length(Other.Length),
		  ReadOpParams(std::move(Other.ReadOpParams)),
		  Coroutine(std::move(Other.Coroutine)),
		  StatusTimer(std::move(Other.StatusTimer)),
		  NumberOfBytesRead(std::move(Other.NumberOfBytesRead)),
		  Destination(std::move(Other.Destination))
	{}

	~StreamReadOp()
	{
		DWORD HandleInfo;

		if (ReadOpParams && GetHandleInformation(ReadOpParams->hEvent, &HandleInfo))
		{
			CloseHandle(ReadOpParams->hEvent);
		}
	}

	template<typename CoroType>
	void operator()(CoroType& Self, std::error_code ec = {})
	{
		if (FileImpl->ShouldCancel())
		{
			Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
			return;
		}

		reenter(Coroutine)
		{
			if (Length == 0)
			{
				Self.complete({});
				return;
			}

			yield asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));
			
			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
										"Begin read of {} bytes from {}", Length, FileImpl->GetPath().string());

			ReadOpParams->hEvent = CreateEvent(NULL, false, false, NULL);

			if (!ReadOpParams->hEvent)
			{
				Self.complete(Modio::make_error_code(Modio::GenericError::CouldNotCreateHandle));
			
				return;
			}

			{
				Modio::FileOffset Offset = FileImpl->Tell();
				ReadOpParams->OffsetHigh = Offset >> 32;
				ReadOpParams->Offset = (DWORD) Offset;
			}
			if (!ReadFile(FileImpl->GetFileHandle(), Buffer.Data(), Length, NULL, ReadOpParams.get()))
			{
				DWORD Error = GetLastError();
				if (Error != ERROR_IO_PENDING)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"StreamReadOp from {} failed, error code = {}",
												FileImpl->GetPath().string(), Error);
					Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
					return;
				}
			}
			else
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Finish read from {}",
											FileImpl->GetPath().string());
				*NumberOfBytesRead = ReadOpParams->InternalHigh;
				FileImpl->Seek(Modio::FileOffset(*NumberOfBytesRead), Modio::Detail::SeekDirection::Forward);

				if (*NumberOfBytesRead < Length)
				{
					Destination.AppendBuffer(Buffer.CopyRange(Buffer.begin(), Buffer.begin() + *NumberOfBytesRead));
				}
				else
				{
					Destination.AppendBuffer(std::move(Buffer));
				}
				Self.complete(std::error_code {});
				
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

			FileImpl->Seek(Modio::FileOffset(*NumberOfBytesRead), Modio::Detail::SeekDirection::Forward);

			if (*NumberOfBytesRead < Length)
			{
				Destination.AppendBuffer(Buffer.CopyRange(Buffer.begin(), Buffer.begin() + *NumberOfBytesRead));
			}
			else
			{
				Destination.AppendBuffer(std::move(Buffer));
			}
			Self.complete(std::error_code {});
			
			return;
		}
	}
};

#include <asio/unyield.hpp>