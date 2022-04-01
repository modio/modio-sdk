/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *   
 */

// MIRRORED TO gdk/file/FileObjectImplementation.h, UPDATE THAT FILE IF THIS IS UPDATED
#pragma once
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
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
			// Strand so that IO ops don't get performed simultaneously
			asio::strand<asio::io_context::executor_type>* Strand;
			std::atomic<bool> OperationInProgress {false};
			std::atomic<std::int32_t> NumWaiters {0};
			asio::steady_timer OperationQueue;
			Modio::FileOffset CurrentSeekOffset = Modio::FileOffset(0);
			bool CancelRequested = false;

		public:
			FileObjectImplementation(asio::io_context& ParentContext, Modio::filesystem::path BasePath)
				: FilePath(),
				  BasePath(BasePath),
				  FileHandle(INVALID_HANDLE_VALUE),
				  Strand(nullptr),
				  OperationQueue(ParentContext, std::chrono::steady_clock::time_point::max()),
				  CurrentSeekOffset(0)
			{}

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
				if (OperationInProgress.exchange(true))
				{
					++NumWaiters;
					OperationQueue.async_wait(
						asio::bind_executor(Modio::Detail::Services::GetGlobalContext().get_executor(),
											[Op = std::move(Operation)](asio::error_code ec) mutable { Op(); }));
				}
				else
				{
					asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Operation));
				}
			}

			void FinishExclusiveOperation()
			{
				OperationInProgress.exchange(false);

				if (NumWaiters > 0)
				{
					--NumWaiters;
					OperationQueue.cancel_one();
				}
			}

			Modio::filesystem::path GetPath()
			{
				return FilePath;
			}

			virtual Modio::ErrorCode Rename(Modio::filesystem::path NewPath) override
			{
				Modio::ErrorCode ec;
				if (FileHandle != INVALID_HANDLE_VALUE)
				{
					CloseHandle(FileHandle);
				}
				Modio::filesystem::rename(FilePath, NewPath, ec);
				if (ec)
				{
					return ec;
				}
				FilePath = NewPath;

				ec = OpenFile(FilePath, false);
				return ec;
			}

			virtual void Truncate(Modio::FileOffset Offset) override
			{
				LARGE_INTEGER Length;
				Length.QuadPart = Offset;
				SetFilePointerEx(FileHandle, Length, NULL, FILE_BEGIN);
				SetEndOfFile(FileHandle);
			}

			virtual std::size_t GetSize() override
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
				return OpenFile(NewFilePath, true);
			}

			Modio::ErrorCode OpenFile(Modio::filesystem::path NewFilePath, bool bOverwrite = false)
			{
				Modio::ErrorCode ec;
				filesystem::create_directories(NewFilePath.parent_path(), ec);
				if (ec)
				{
					return ec;
				}

				this->FilePath = NewFilePath;
				FileHandle = ::CreateFileW(this->FilePath.generic_wstring().c_str(), GENERIC_READ | GENERIC_WRITE,
										   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
										   bOverwrite ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
				if (FileHandle != INVALID_HANDLE_VALUE)
				{
					return std::error_code {};
				}
				else
				{
					DWORD Error = GetLastError();
					// post failure
					return Modio::ErrorCode(Error, std::system_category());
				}
			}

			HANDLE GetFileHandle()
			{
				return FileHandle;
			}
		};
	} // namespace Detail
} // namespace Modio
