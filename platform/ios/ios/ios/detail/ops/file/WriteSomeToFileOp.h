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
#include "file/FileObjectImplementation.h"
#include "ios/FileSharedState.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/timer/ModioTimer.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		/// @brief Unified operation for writing a fixed quantity of data to file.
		/// If Offset is an empty optional, will write at the current seek offset specified by the file implementation
		/// otherwise will write at the specified offset.
		/// This avoids us needing to implement streamed reads or writes separately from offset-based ones
		class WriteSomeToFileOp
		{
		public:
			WriteSomeToFileOp(std::shared_ptr<Modio::Detail::FileObjectImplementation> IOObject,
							  std::shared_ptr<Modio::Detail::FileSharedState> SharedState,
							  Modio::Optional<Modio::FileOffset> Offset, Modio::Detail::Buffer Buffer)
				: Buffer(std::move(Buffer)),
				  FileImpl(IOObject),
				  FileOffset(Offset),
				  SharedState(SharedState),
				  BufferSize(this->Buffer.GetSize()) {};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				Modio::Optional<Modio::ErrorCode> CreationEC = {};
				// If the shared state goes out of scope for whatever reason, treat it as a cancellation
				std::shared_ptr<Modio::Detail::FileSharedState> PinnedSharedState = SharedState.lock();
				if (FileImpl->ShouldCancel() || (PinnedSharedState == nullptr))
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}
				if (FileImpl->GetFileMode() == Modio::Detail::FileMode::ReadOnly)
				{
					Self.complete(Modio::make_error_code(Modio::FilesystemError::NoPermission));
					return;
				}

				reenter(CoroutineState)
				{
					yield asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));

					if (BufferSize == 0)
					{
						Self.complete({});
						return;
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
												"Begin write for {}, File Descriptor {}, expected size: {}, Offset: {}",
												FileImpl->GetPath().u8string(), FileImpl->GetFileHandle(), BufferSize,
												FileOffset.has_value() ? static_cast<std::uint32_t>(FileOffset.value()) : 0);

					// SubmitWrite could fail with system errors.
					CreationEC = PinnedSharedState->SubmitWrite(FileImpl->GetFileHandle(), std::move(Buffer),
																FileOffset.value_or(FileImpl->Tell()));

					if (CreationEC.has_value() == true)
					{
						Self.complete(CreationEC.value());
						return;
					}

					while ((WriteResult = PinnedSharedState->IOCompleted(FileImpl->GetFileHandle())).first == false)
					{
						StatusTimer.ExpiresAfter(Modio::Detail::Constants::Configuration::PollInterval);
						yield StatusTimer.WaitAsync(std::move(Self));
					}

					// If the result is falsey(ie no errors)
					if (WriteResult.second.has_value() == false)
					{
						// If we didn't specify a specific offset then this is a streamed write which alters the
						// position in the file
						if (!FileOffset)
						{
							FileImpl->Seek(BufferSize, SeekDirection::Forward);
						}

						Self.complete({});
						return;
					}
					else
					{
						Self.complete(WriteResult.second.value());
						return;
					}
				}
			}

		private:
			asio::coroutine CoroutineState {};
			Modio::Detail::Buffer Buffer;
			std::shared_ptr<Modio::Detail::FileObjectImplementation> FileImpl {};
			Modio::Optional<Modio::FileOffset> FileOffset {};
			std::weak_ptr<Modio::Detail::FileSharedState> SharedState {};
			Modio::FileOffset BufferSize {};
			std::pair<bool, Modio::Optional<Modio::ErrorCode>> WriteResult {};
			Modio::Detail::Timer StatusTimer {};
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
