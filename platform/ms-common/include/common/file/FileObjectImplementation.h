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
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioOperationQueue.h"
#include "modio/detail/file/IFileObjectImplementation.h"
#include <atomic>
#include <chrono>
#include <fileapi.h>
namespace Modio
{
	namespace Detail
	{
		class FileObjectImplementation : public Modio::Detail::IFileObjectImplementation
		{
			Modio::filesystem::path FilePath;
			Modio::filesystem::path BasePath;
			HANDLE FileHandle;
			Modio::Detail::FileMode FileMode;
			// Strand so that IO ops don't get performed simultaneously
			asio::strand<asio::io_context::executor_type>* Strand;
			std::atomic<bool> OperationInProgress {false};
			std::atomic<std::int32_t> NumWaiters {0};
			// asio::steady_timer OperationQueue;
			std::shared_ptr<Modio::Detail::OperationQueue> OperationQueue;
			std::unique_ptr<Modio::Detail::OperationQueue::Ticket> CurrentTicket;
			Modio::FileOffset CurrentSeekOffset = Modio::FileOffset(0);
			bool CancelRequested = false;

		public:
			FileObjectImplementation(asio::io_context& ParentContext, Modio::filesystem::path BasePath)
				: FilePath(),
				  BasePath(BasePath),
				  FileHandle(INVALID_HANDLE_VALUE),
				  Strand(nullptr),
				  // OperationQueue(ParentContext, std::chrono::steady_clock::time_point::max()),

				  CurrentSeekOffset(0)
			{
				OperationQueue = std::make_shared<Modio::Detail::OperationQueue>(ParentContext);
			}

			~FileObjectImplementation()
			{
				Destroy();
			}
			void Destroy()
			{
				if (FileHandle != INVALID_HANDLE_VALUE)
				{
					CloseHandle(FileHandle);
					FileHandle = INVALID_HANDLE_VALUE;
				}
			}
			void Close()
			{
				Destroy();
			}

			void CancelAll()
			{
				CancelRequested = true;
			}

			bool ShouldCancel()
			{
				return CancelRequested;
			}

			template<typename OperationType>
			void BeginExclusiveOperation(OperationType&& Operation)
			{
				CurrentTicket = std::make_unique<Modio::Detail::OperationQueue::Ticket>(OperationQueue);
				CurrentTicket->WaitForTurnAsync(std::forward<OperationType>(Operation));
				/*
								if (OperationInProgress.exchange(true))
								{
									++NumWaiters;
									OperationQueue.async_wait(
										asio::bind_executor(Modio::Detail::Services::GetGlobalContext().get_executor(),
															[Op = std::move(Operation)](asio::error_code ec) mutable {
				   Op(); }));
								}
								else
								{
									asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
				   std::move(Operation));
								}
				*/
			}

			void FinishExclusiveOperation()
			{
				/*OperationInProgress.exchange(false);

				if (NumWaiters > 0)
				{
					--NumWaiters;
					OperationQueue.cancel_one();
				}*/
				CurrentTicket.reset();
			}

			Modio::filesystem::path GetPath()
			{
				return FilePath;
			}

			virtual Modio::ErrorCode Rename(Modio::filesystem::path NewPath) override
			{
				Modio::ErrorCode ec;

				Close();

				Modio::filesystem::rename(FilePath, NewPath, ec);
				if (ec)
				{
					return ec;
				}
				FilePath = NewPath;

				ec = OpenFile(FilePath, Modio::Detail::FileMode::ReadWrite, false);
				return ec;
			}

			virtual Modio::ErrorCode Truncate(Modio::FileOffset Offset) override
			{
				LARGE_INTEGER Length;
				Length.QuadPart = Offset;
				bool Result = SetFilePointerEx(FileHandle, Length, NULL, FILE_BEGIN);
				if (Result == true)
				{
					SetEndOfFile(FileHandle);
					return {};
				}
				else
				{
					DWORD Error = GetLastError();
					// post failure
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Truncate file error code = {}", Error);
					return Modio::make_error_code(Modio::FilesystemError::WriteError);
				}
			}

			virtual std::uint64_t GetSize() override
			{
				if (FileHandle != INVALID_HANDLE_VALUE)
				{
					LARGE_INTEGER FileSize;
					GetFileSizeEx(FileHandle, &FileSize);
					return FileSize.QuadPart;
				}
				return INVALID_FILE_SIZE;
			}

			virtual void Seek(Modio::FileOffset Offset, Modio::Detail::SeekDirection Direction) override
			{
				switch (Direction)
				{
					case Modio::Detail::SeekDirection::Absolute:
						CurrentSeekOffset = Offset;
						break;
					case Modio::Detail::SeekDirection::Forward:
						CurrentSeekOffset += Offset;
						break;
					case Modio::Detail::SeekDirection::Backward:
						if (CurrentSeekOffset < Offset)
						{
							CurrentSeekOffset = FileOffset(0);
						}
						else
						{
							CurrentSeekOffset -= Offset;
						}
				}
			}

			Modio::FileOffset Tell()
			{
				return CurrentSeekOffset;
			}

			void SetFileStrand(asio::strand<asio::io_context::executor_type>& FileStrand)
			{
				Strand = &FileStrand;
			}

			asio::strand<asio::io_context::executor_type>& GetFileStrand()
			{
				return *Strand;
			}

			Modio::ErrorCode CreateFile(Modio::filesystem::path NewFilePath)
			{
				return OpenFile(NewFilePath, Modio::Detail::FileMode::ReadWrite, true);
			}

			Modio::Detail::FileMode GetFileMode() override
			{
				return FileMode;
			}

			Modio::ErrorCode OpenFile(Modio::filesystem::path NewFilePath, Modio::Detail::FileMode NewMode,
									  bool bOverwrite = false)
			{
				if (FileHandle != INVALID_HANDLE_VALUE)
				{
					Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
												"Attempted to open an already opened file");
					return Modio::make_error_code(Modio::GenericError::CouldNotCreateHandle);
				}

				Modio::ErrorCode ec;
				filesystem::create_directories(NewFilePath.parent_path(), ec);
				if (ec)
				{
					return ec;
				}

				this->FilePath = NewFilePath;
				this->FileMode = NewMode;

				DWORD Access;
				if (FileMode == Modio::Detail::FileMode::ReadWrite)
				{
					Access = GENERIC_READ | GENERIC_WRITE;
				}
				else
				{
					Access = GENERIC_READ;
				}

				FileHandle =
					::CreateFileW(this->FilePath.generic_wstring().c_str(), Access, FILE_SHARE_READ | FILE_SHARE_WRITE,
								  NULL, bOverwrite ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
				if (FileHandle != INVALID_HANDLE_VALUE)
				{
					return std::error_code {};
				}
				else
				{
					DWORD Error = GetLastError();
					// post failure
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"FileObjectImplementation OpenFile {} failed, error code = {}",
												NewFilePath.string(), Error);
					return Modio::make_error_code(Modio::FilesystemError::NoPermission);
				}
			}

			HANDLE GetFileHandle()
			{
				return FileHandle;
			}
		};
	} // namespace Detail
} // namespace Modio
