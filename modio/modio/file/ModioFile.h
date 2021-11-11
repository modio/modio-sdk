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
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/file/ModioFileService.h"

namespace Modio
{
	namespace Detail
	{
		class File : public asio::basic_io_object<Modio::Detail::FileService>
		{
			Modio::filesystem::path FilePath;
			asio::strand<asio::io_context::executor_type> FileStrand;

		public:
			explicit File(Modio::filesystem::path FilePath, bool bOverwriteExisting = false)
				: asio::basic_io_object<Modio::Detail::FileService>(Modio::Detail::Services::GetGlobalContext()),
				  FilePath(FilePath),
				  FileStrand(asio::make_strand(Modio::Detail::Services::GetGlobalContext()))
			{
				get_implementation()->SetFileStrand(FileStrand);
				std::error_code ec = get_implementation()->OpenFile(FilePath, bOverwriteExisting);
				if (ec)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Error code {} while trying to open {}", ec.value(), FilePath.string());
				}
			}

			explicit File(asio::io_context& Context, Modio::filesystem::path FilePath, bool bOverWriteExisting = false)
				: asio::basic_io_object<Modio::Detail::FileService>(Context),
				  FilePath(FilePath),
				  FileStrand(asio::make_strand(Context))
			{
				get_implementation()->SetFileStrand(FileStrand);
				std::error_code ec = get_implementation()->OpenFile(FilePath, bOverWriteExisting);
				if (ec)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Error code {} while trying to open {}", ec.value(), FilePath.string());
				}
			};

			File(Modio::Detail::File&& Other)
				: asio::basic_io_object<Modio::Detail::FileService>(std::move(Other)),
				  FilePath(std::move(Other.FilePath)),
				  FileStrand(std::move(Other.FileStrand)) {};

			std::size_t GetFileSize()
			{
				return get_implementation()->GetSize();
			}

			filesystem::path GetPath()
			{
				return get_implementation()->GetPath();
			}

			Modio::ErrorCode Rename(Modio::filesystem::path NewPath)
			{
				return get_implementation()->Rename(NewPath);
			}

			void Truncate(Modio::FileOffset Offset)
			{
				get_implementation()->Truncate(Offset);
			}
			void Seek(Modio::FileOffset Offset,
					  Modio::Detail::SeekDirection Direction = Modio::Detail::SeekDirection::Absolute)
			{
				get_implementation()->Seek(Offset, Direction);
			}

			Modio::FileOffset Tell()
			{
				return get_implementation()->Tell();
			}

			template<typename CompletionTokenType>
			auto WriteSomeAtAsync(std::uintmax_t Offset, Modio::Detail::Buffer Buffer, CompletionTokenType&& Token)
			{
				return get_service().WriteSomeAtAsync(get_implementation(), Offset, std::move(Buffer),
													  std::forward<CompletionTokenType>(std::move(Token)));
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(std::uintmax_t Offset, std::uintmax_t Length, CompletionTokenType&& Token)
			{
				return get_service().ReadSomeAtAsync(get_implementation(), Offset, Length,
													 std::forward<CompletionTokenType>(std::move(Token)));
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(std::uintmax_t Offset, std::uintmax_t MaxBytesToRead,
								 Modio::Detail::DynamicBuffer Destination, CompletionTokenType&& Token)
			{
				return get_service().ReadSomeAtAsync(get_implementation(), Offset, MaxBytesToRead, Destination,
													 std::forward<CompletionTokenType>(std::move(Token)));
			}

			template<typename CompletionTokenType>
			auto ReadAsync(std::uintmax_t MaxBytesToRead, Modio::Detail::DynamicBuffer Destination,
						   CompletionTokenType&& Token)
			{
				return get_service().ReadAsync(get_implementation(), MaxBytesToRead, Destination,
											   std::forward<CompletionTokenType>(std::move(Token)));
			}

			template<typename CompletionTokenType>
			auto WriteAsync(Modio::Detail::Buffer Buffer, CompletionTokenType&& Token)
			{
				return get_service().WriteAsync(get_implementation(), std::move(Buffer),
												std::forward<CompletionTokenType>(std::move(Token)));
			}
		};
	} // namespace Detail
} // namespace Modio
