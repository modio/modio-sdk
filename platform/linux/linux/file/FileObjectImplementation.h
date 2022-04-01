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
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/file/IFileObjectImplementation.h"
#include <atomic>
#include <chrono>
namespace Modio
{
	namespace Detail
	{
		class FileObjectImplementation : public Modio::Detail::IFileObjectImplementation
		{
			constexpr static int InvalidFileDescriptor = -1;
			Modio::filesystem::path FilePath;
			Modio::filesystem::path BasePath;
			int FileDescriptor;
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
				  FileDescriptor(InvalidFileDescriptor),
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
				if (FileDescriptor != InvalidFileDescriptor)
				{
					close(FileDescriptor);
					FileDescriptor = InvalidFileDescriptor;
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
			
			Modio::filesystem::path GetPath()
			{
				return FilePath;
			}

			virtual Modio::ErrorCode Rename(Modio::filesystem::path NewPath) override
			{
				Modio::ErrorCode ec;
				if (FileDescriptor != InvalidFileDescriptor)
				{
					close(FileDescriptor);
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
				Modio::ErrorCode ec;
				Modio::filesystem::resize_file(FilePath, Offset, ec);
			}

			virtual std::size_t GetSize() override
			{
				Modio::ErrorCode ec;
				return Modio::filesystem::file_size(FilePath, ec);
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

			Modio::ErrorCode OpenFile(Modio::filesystem::path Path, bool bOverwrite = false)
			{
				Modio::ErrorCode ec;
				filesystem::create_directories(Path.parent_path(), ec);
				if (ec)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Could not create parent directories for file {}", Path.u8string());
					return ec;
				}

				this->FilePath = Path;
				FileDescriptor = open(this->FilePath.generic_u8string().c_str(),
									  O_RDWR | O_CREAT | O_NONBLOCK | (bOverwrite ? O_TRUNC : 0), S_IRUSR | S_IWUSR);

				if (FileDescriptor != InvalidFileDescriptor)
				{
					return std::error_code {};
				}
				else
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
												"Re-attempting open of file {} in read-only mode", Path.u8string());
					FileDescriptor =
						open(this->FilePath.generic_u8string().c_str(),
							 O_RDONLY | O_NONBLOCK, S_IRUSR);
					if (FileDescriptor != InvalidFileDescriptor)
					{
						return std::error_code {};
					}
					else
					{
						return Modio::ErrorCode(errno, std::system_category());
					}
				}
			}

			int GetFileHandle()
			{
				return FileDescriptor;
			}
		};
	} // namespace Detail
} // namespace Modio
