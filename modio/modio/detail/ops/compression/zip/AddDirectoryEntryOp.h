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
#include "modio/detail/compression/zip/ZipStructures.h"
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
                    LocalHeaderOffset = OutputFile->Tell();

					// Marshal fields into our local file header's data buffer
					Modio::Detail::TypedBufferWrite(Constants::ZipTag::LocalDirectoryHeader, LocalFileHeaderBuffer, 0) // header signature
						.FollowedBy<uint16_t>(Constants::ZipTag::ZipVersion) // Minimum version to extract
						.FollowedBy<uint16_t>(0) // General Purpose bitflag
						.FollowedBy<uint16_t>(Constants::ZipTag::Deflate) // Compression Method
						.FollowedBy<uint16_t>(0) // Last Modified time
						.FollowedBy<uint16_t>(0) // Last Modified date
						.FollowedBy<uint32_t>(0) // CRC-32 of uncompressed data
						.FollowedBy<uint32_t>(0) // Compressed size
						.FollowedBy<uint32_t>(0) // Uncompressed size
						.FollowedBy<uint16_t>((std::uint16_t) DirectoryPath.generic_u8string().size()) // Size of filename
                        .FollowedBy<uint16_t>(0);

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
					// Set the file data offset to "LocalHeaderOffset" to easily allocate the LDHS in the file
					ArchiveFile->AddEntry(DirectoryPath.generic_u8string(),
                                          LocalHeaderOffset, 0, 0,
										  ArchiveFileImplementation::CompressionMethod::Store, 0, true);
					Self.complete({});
					return;
				}
			}

		private:
            std::shared_ptr<Modio::Detail::ArchiveFileImplementation> ArchiveFile;
			std::unique_ptr<Modio::Detail::File> OutputFile;
			Modio::filesystem::path DirectoryPath;
			Modio::Detail::Buffer LocalFileHeaderBuffer;
            // Use this to identify where the "Local directory header signature" in the zip file for this
            // file is located
            Modio::FileOffset LocalHeaderOffset;
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
