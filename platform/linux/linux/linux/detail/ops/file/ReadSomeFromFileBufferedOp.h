/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once
#include "file/FileObjectImplementation.h"
#include "linux/FileSharedState.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		/// @brief Unified operation for reading a fixed quantity of data to file.
		/// If Offset is an empty optional, will read at the current seek offset specified by the file implementation
		/// otherwise will read at the specified offset.
		/// This avoids us needing to implement streamed reads or writes separately from offset-based ones
		class ReadSomeFromFileBufferedOp
		{
		public:
			ReadSomeFromFileBufferedOp(std::shared_ptr<Modio::Detail::FileObjectImplementation> IOObject,
									   std::shared_ptr<Modio::Detail::FileSharedState> SharedState,
									   Modio::Optional<Modio::FileOffset> Offset, Modio::FileSize MaxBytesToRead,
									   Modio::Detail::DynamicBuffer Destination)
				: FileImpl(IOObject),
				  MaxBytesToRead(MaxBytesToRead),
				  FileOffset(Offset),
				  SharedState(SharedState),
				  Destination(Destination) {};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				Modio::Optional<Modio::ErrorCode> CurrentErrorCode = {};
				// If the shared state goes out of scope for whatever reason, treat it as a cancellation
				std::shared_ptr<Modio::Detail::FileSharedState> PinnedState = SharedState.lock();
				if (FileImpl->ShouldCancel() || (PinnedState == nullptr))
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				reenter(CoroutineState)
				{
					yield asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));
					Modio::Detail::Logger().Log(
						Modio::LogLevel::Trace, Modio::LogCategory::File,
						"Begin buffered read for {}, File Descriptor {}, expected size: {}, Offset: {}",
						FileImpl->GetPath().string(), FileImpl->GetFileHandle(), MaxBytesToRead,
						FileOffset.has_value() ? FileOffset.value() : 0);
					CurrentErrorCode = PinnedState->SubmitRead(FileImpl->GetFileHandle(), MaxBytesToRead,
															   FileOffset.value_or(FileImpl->Tell()));

					if (CurrentErrorCode.has_value() == true)
					{
						Self.complete(CurrentErrorCode.value());
						return;
					}

					while ((ReadResult = PinnedState->IOCompleted(FileImpl->GetFileHandle())).first == false)
					{
						if (PollTimer == nullptr)
						{
							PollTimer = std::make_unique<asio::steady_timer>(
								Modio::Detail::Services::GetGlobalContext().get_executor());
						}
						PollTimer->expires_after(std::chrono::milliseconds(1));
						yield PollTimer->async_wait(std::move(Self));
					}

					CurrentErrorCode = ReadResult.second;

					if (CurrentErrorCode.has_value() == true &&
						CurrentErrorCode.value() != Modio::GenericError::EndOfFile)
					{
						// If the caller makes a call with "out-of-bounds" parameters, like an offset larger than
						// the file, it will return the error along a 0 buffer. This satisfies the
						// ParseArchiveContentsOp.
						Modio::Detail::Buffer EmptyData(MaxBytesToRead);
						Destination.AppendBuffer(std::move(EmptyData));
						Self.complete(CurrentErrorCode.value());
						return;
					}

					ReadBuffer = PinnedState->RetrieveReadBuffer(FileImpl->GetFileHandle());

					if (ReadBuffer.has_value())
					{
						// If we didn't specify a specific offset then this is a streamed read which alters the
						// position in the file
						if (!FileOffset)
						{
							FileImpl->Seek(Modio::FileOffset(ReadBuffer->GetSize()), SeekDirection::Forward);
						}

						// Partial reads might return an "EndOfFile" error. When that happens, we append the partial
						// buffer and state that error if happens.
						if (CurrentErrorCode.has_value() == true)
						{
							if (FileOffset.has_value() == false)
							{
								Destination.AppendBuffer(std::move(ReadBuffer.value()));
								Self.complete(CurrentErrorCode.value());
							}
							else
							{
								// This condition happens when the caller reads a section of a file and provides an
								// offset respective to the original file. The IO operation has access to the whole
								// file, therefore only the OffsetData is returned in the Destination
								std::size_t OffsetVal = FileOffset.value();
								std::size_t MaxBytes = MIN(MaxBytesToRead, ReadBuffer.value().GetSize());
								Modio::Detail::Buffer OffsetData =
									ReadBuffer->CopyRange(OffsetVal, OffsetVal + MaxBytes);
								Destination.AppendBuffer(std::move(OffsetData));
								Self.complete({});
							}
						}
						else
						{
							Destination.AppendBuffer(std::move(ReadBuffer.value()));
							Self.complete({});
						}

						return;
					}
					else
					{
						// Strange error where OS reports no error but we have no buffer
						Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
						return;
					}
				}
			}

		private:
			Modio::Optional<Modio::Detail::Buffer> ReadBuffer;
			asio::coroutine CoroutineState;
			std::shared_ptr<Modio::Detail::FileObjectImplementation> FileImpl;
			Modio::FileSize MaxBytesToRead;
			Modio::Optional<Modio::FileOffset> FileOffset;
			std::weak_ptr<Modio::Detail::FileSharedState> SharedState;
			std::pair<bool, Modio::Optional<Modio::ErrorCode>> ReadResult;
			std::unique_ptr<asio::steady_timer> PollTimer;
			Modio::Detail::DynamicBuffer Destination;
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
