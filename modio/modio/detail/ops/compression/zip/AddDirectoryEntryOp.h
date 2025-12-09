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

// For reference: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT

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
				  LocalFileHeaderBuffer(Constants::ZipTag::LocalFileHeaderSize +
										DirectoryPath.generic_u8string().size())
			{
				OutputFile = std::make_unique<Modio::Detail::File>(ArchiveFile->FilePath,
																   Modio::Detail::FileMode::ReadWrite, false);
				FileName = Modio::ToModioString(DirectoryPath.generic_u8string());
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					// Determine the offset for this new entry
					// The central directory requires this value to locate this entry
					OutputFile->Seek(Modio::FileOffset(OutputFile->GetFileSize()));
					LocalHeaderOffset = OutputFile->Tell();

					// Marshal fields of fixed sizes into our local file header's data buffer
					// Header signature
					Modio::Detail::TypedBufferWrite(Constants::ZipTag::LocalFileHeaderSignature, LocalFileHeaderBuffer,
													0)
						// Minimum version to extract
						.FollowedBy<uint16_t>(Constants::ZipTag::ZipVersion)
						// General Purpose bit-flag
						.FollowedBy<uint16_t>(0)
						// Compression Method
						.FollowedBy<uint16_t>(Constants::ZipTag::Store)
						// Last Modified time
						.FollowedBy<uint16_t>(0)
						// Last Modified date
						.FollowedBy<uint16_t>(0)
						// CRC-32 of uncompressed data
						.FollowedBy<uint32_t>(0)
						// Compressed size
						.FollowedBy<uint32_t>(0)
						// Uncompressed size
						.FollowedBy<uint32_t>(0)
						// File name length
						.FollowedBy<uint16_t>(std::uint16_t(FileName.size()))
						// Extra field length
						.FollowedBy<uint16_t>(0);

					// Manually write (variable sized) file name
					std::copy(FileName.begin(), FileName.end(),
							  LocalFileHeaderBuffer.Data() + Constants::ZipTag::LocalFileHeaderSize);

					// Write out this local file header
					yield OutputFile->WriteAsync(std::move(LocalFileHeaderBuffer), std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// Add this entry to the archive file object
					// These details will be written to the central directory
					ArchiveFile->AddEntry(FileName, LocalHeaderOffset, 0, 0,
										  ArchiveFileImplementation::CompressionMethod::Store, 0, true);

					// Close file handle
					OutputFile.reset();

					Self.complete({});
					return;
				}
			}

		private:
			std::shared_ptr<Modio::Detail::ArchiveFileImplementation> ArchiveFile;
			std::unique_ptr<Modio::Detail::File> OutputFile;
			Modio::filesystem::path DirectoryPath;
			Modio::Detail::Buffer LocalFileHeaderBuffer;
			Modio::FileOffset LocalHeaderOffset;
			ModioAsio::coroutine CoroutineState;
			std::string FileName;
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
