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
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
#include "modio/file/ModioFile.h"
#include <algorithm>
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class AddDirectoryEntryOp
		{
		public:
			AddDirectoryEntryOp(std::shared_ptr<Modio::Detail::ArchiveFileImplementation> ArchiveFile,
								Modio::filesystem::path DirectoryPath)
				: ArchiveFile(ArchiveFile),
				  DirectoryPath(DirectoryPath),
				  LocalFileHeaderBuffer(GetLocalFileHeaderSize())
			{
				OutputFile = std::make_unique<Modio::Detail::File>(ArchiveFile->FilePath, false);
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					OutputFile->Seek(Modio::FileOffset(OutputFile->GetFileSize()));

					// Marshal fields into our local file header's data buffer
					Modio::Detail::TypedBufferWrite(LocalHeaderMagic, LocalFileHeaderBuffer, 0) // header signature
						.FollowedBy<uint16_t>(20) // Minimum version to extract
						.FollowedBy<uint16_t>(0) // General Purpose bitflag
						.FollowedBy<uint16_t>(8) // Compression Method
						.FollowedBy<uint16_t>(0) // Last Modified time
						.FollowedBy<uint16_t>(0) // Last Modified date
						.FollowedBy<uint32_t>(0) // CRC32 of input data
						.FollowedBy<uint32_t>(0) // Size of output data
						.FollowedBy<uint32_t>(0) // Size of Input Data
						.FollowedBy<uint32_t>((std::uint32_t) DirectoryPath.generic_u8string().size()) // Size of filename
						.FollowedBy<uint32_t>(
							0); // Size of extra field (currently not in use, will change when zip64 support is added)

					// TypedBufferWrite doesn't currently handle strings so manually marshal the filename
					{
						std::string FileName = DirectoryPath.generic_u8string();
						std::copy(FileName.begin(), FileName.end(), LocalFileHeaderBuffer.Data() + 30);
					}

					// Write the local file header
					yield OutputFile->WriteAsync(std::move(LocalFileHeaderBuffer), std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// Add the entry to the archive file so that it knows what to put in the central directory when it
					// is finalized
					// Set the file data offset to 0 because we have no file data at all, just the entry itself
					ArchiveFile->AddEntry(DirectoryPath.generic_u8string(), 0, 0, 0,
										  ArchiveFileImplementation::CompressionMethod::Store, 0, true);
					Self.complete({});
					return;
				}
			}

		private:
            // This variable had troubles as a constexpr when compiling on macOS + g++11
			const uint32_t LocalHeaderMagic = 0x04034b50;
			std::shared_ptr<Modio::Detail::ArchiveFileImplementation> ArchiveFile;
			std::unique_ptr<Modio::Detail::File> OutputFile;
			Modio::filesystem::path DirectoryPath;
			Modio::Detail::Buffer LocalFileHeaderBuffer;
			asio::coroutine CoroutineState;
			/// @brief Helper function so when we add zip64 support we can easily change how much space to allocate for
			/// the local header
			/// @return
			std::size_t GetLocalFileHeaderSize()
			{
				return 30 + DirectoryPath.generic_u8string().size();
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
