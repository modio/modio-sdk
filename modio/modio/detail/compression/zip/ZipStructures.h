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
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioCompilerMacros.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include <cstdint>
#include <vector>


namespace Modio
{
	namespace Detail
	{
		MODIO_DISABLE_WARNING_PUSH
		MODIO_DISABLE_WARNING_UNUSED_CONST_VARIABLE
		namespace Constants
		{
			namespace ZipTag
			{
				// Zip64 Extended information Extra Field
				constexpr static uint16_t Zip64ExtraFieldSignature = 0x0001;
				// Local directory header signature
				constexpr static uint32_t LocalFileHeaderSignature = 0x04034b50;
				// (Optional) Data descriptor ID
				constexpr static uint32_t DataDescriptorSignature = 0x08074b50;
				// Central directory file header signature
				constexpr static uint32_t CentralFileHeaderSignature = 0x02014b50;
				// End of central directory signature
				constexpr static uint32_t EndCentralDirectorySignature = 0x06054b50;
				// Zip64 End of central directory signature
				constexpr static uint32_t Zip64EndCentralDirectorySignature = 0x06064b50;
				// End of central directory locator
				constexpr static uint32_t Zip64EndCentralDirectoryLocatorSignature = 0x07064b50;

				// Size (bytes) of various headers

				// Local directory file header size, excluding variable sized fields
				constexpr static uint32_t LocalFileHeaderSize = 30;
				// Extra field size for a local directory file header
				// Local directory has no fields for disk number or offset
				constexpr static uint16_t Zip64LocalFileExtraFieldSize = 20;
				// Data descriptor size
				constexpr static uint32_t DataDescriptorSize = 16;
				// Zip64 Data descriptor size
				constexpr static uint32_t Zip64DataDescriptorSize = 24;
				// Size of a central directory file header, excluding variable sized fields
				constexpr static uint8_t CentralFileHeaderFixedSize = 46;
				// Size of Zip64 End Of Central Directory, excluding variable sized fields
				constexpr static uint32_t Zip64EndCentralDirectoryHeaderFixedSize = 56;
				// Size of Zip64 End of central directory locator
				constexpr static uint32_t Zip64EndCentralDirectoryLocatorSize = 20;
				// Size of End of Central Directory Record, excluding variable sized field
				constexpr static uint8_t EndOfCentralDirectoryFixedSize = 22;

				// Other useful constants

				// Signal used to denote out-of-bounds
				constexpr static uint32_t MAX32 = 0xffffffff;
				// Signal used to denote out-of-bounds
				constexpr static uint16_t MAX16 = 0xffff;
				// Compression with STORE
				constexpr static uint16_t Store = 0;
				// Compression with DEFLATE
				constexpr static uint16_t Deflate = 8;
				// Min version for zip32 with Deflate compression
				constexpr static uint16_t ZipVersion = 20;
				// Min version required for Zip64 decompression
				constexpr static uint16_t Zip64Version = 45;

			} // namespace ZipTag
		} // namespace Constants
		MODIO_DISABLE_WARNING_POP

		struct ZipStructures
		{
			static bool ValidSignatureSize(const uint16_t& Size)
			{
				return (Size == 8) || (Size == 16) || (Size == 24) || (Size == 28);
			}

			static std::tuple<std::uintmax_t, bool, std::uint64_t> FindOffsetInBuffer(Modio::Detail::Buffer& Chunk,
																					  bool PreZip64)
			{
				MODIO_PROFILE_SCOPE(ArchiveFindOffsetInBuffer);

				bool IsZip64 = false;
				std::uintmax_t LocalPosition = Chunk.GetSize() - 4;
				std::uintmax_t MagicOffset = 0;
				std::uint32_t BytesPastEOCD = 0;
				std::uint64_t Zip64EndCentralDirectoryLocation = 0;

				for (; LocalPosition > 0; LocalPosition--)
				{
					uint32_t Value = Modio::Detail::TypedBufferRead<std::uint32_t>(Chunk, LocalPosition);

					// Look for the standard EOCD signature, which is always included
					if (Value == Constants::ZipTag::EndCentralDirectorySignature && PreZip64 == false)
					{
						MagicOffset = LocalPosition;
					}
					// Look for the Zip64 locator to quickly find the Zip64 EOCD location
					else if (Value == Constants::ZipTag::Zip64EndCentralDirectoryLocatorSignature)
					{
						IsZip64 = true;
						// Check that all data is on a single disk, and find Zip64EOCD location
						std::uint32_t StartDisk =
							Modio::Detail::TypedBufferRead<std::uint32_t>(Chunk, LocalPosition + 4);
						if (StartDisk == 0)
						{
							Zip64EndCentralDirectoryLocation =
								Modio::Detail::TypedBufferRead<std::uint64_t>(Chunk, LocalPosition + 8);
							break;
						}
					}
					// Look for the Zip64 EOCD itself
					else if (Value == Constants::ZipTag::Zip64EndCentralDirectorySignature)
					{
						IsZip64 = true;
						MagicOffset = LocalPosition;
						break;
					}
					// Even if we've already found the MagicOffset for a Zip32 file, we should still check for a Zip64
					// locator to account for edge cases where it may still be included. This check ensures we continue
					// to look for Zip64 information, but only for the expected distance past the Zip32 EOCD
					if (!PreZip64 && MagicOffset != 0)
					{
						if (BytesPastEOCD > Constants::ZipTag::Zip64EndCentralDirectoryLocatorSize)
						{
							break;
						}
						BytesPastEOCD++;
					}
				}
				return std::make_tuple(MagicOffset, IsZip64, Zip64EndCentralDirectoryLocation);
			}

			static std::tuple<uint64_t, uint64_t, uint64_t> ReadCentralDirectory(Modio::Detail::Buffer& Chunk,
																				 bool IsZip64)
			{
				uint64_t NumberOfRecords = 0;
				uint64_t CentralDirectorySize = 0;
				uint64_t CentralDirectoryOffset = 0;

				if (IsZip64 == true)
				{
					NumberOfRecords = Modio::Detail::TypedBufferRead<std::uint64_t>(Chunk, 32);
					CentralDirectorySize = Modio::Detail::TypedBufferRead<std::uint64_t>(Chunk, 40);
					CentralDirectoryOffset = Modio::Detail::TypedBufferRead<std::uint64_t>(Chunk, 48);
				}
				else
				{
					NumberOfRecords = Modio::Detail::TypedBufferRead<std::uint16_t>(Chunk, 10);
					CentralDirectorySize = Modio::Detail::TypedBufferRead<std::uint32_t>(Chunk, 12);
					CentralDirectoryOffset = Modio::Detail::TypedBufferRead<std::uint32_t>(Chunk, 16);
				}

				return std::make_tuple(NumberOfRecords, CentralDirectorySize, CentralDirectoryOffset);
			}

			static std::tuple<ArchiveFileImplementation::ArchiveEntry, std::uint64_t, Modio::ErrorCode> ArchiveParse(
				Modio::Detail::Buffer& FileChunk, std::uint64_t CurrentRecordOffset)
			{
				std::uint16_t CompressionMethod =
					Modio::Detail::TypedBufferRead<std::uint16_t>(FileChunk, CurrentRecordOffset + 10);
				std::uint32_t InputCRC =
					Modio::Detail::TypedBufferRead<std::uint32_t>(FileChunk, CurrentRecordOffset + 16);
				std::uint64_t CompressedSize =
					Modio::Detail::TypedBufferRead<std::uint32_t>(FileChunk, CurrentRecordOffset + 20);
				std::uint64_t UncompressedSize =
					Modio::Detail::TypedBufferRead<std::uint32_t>(FileChunk, CurrentRecordOffset + 24);
				std::uint16_t FileNameLength =
					Modio::Detail::TypedBufferRead<std::uint16_t>(FileChunk, CurrentRecordOffset + 28);
				std::uint16_t ExtraFieldLength =
					Modio::Detail::TypedBufferRead<std::uint16_t>(FileChunk, CurrentRecordOffset + 30);
				std::uint16_t CommentLength =
					Modio::Detail::TypedBufferRead<std::uint16_t>(FileChunk, CurrentRecordOffset + 32);
				std::uint32_t ExternalAttributes =
					Modio::Detail::TypedBufferRead<std::uint32_t>(FileChunk, CurrentRecordOffset + 36);
				std::uint64_t LocalHeaderOffset =
					Modio::Detail::TypedBufferRead<std::uint32_t>(FileChunk, CurrentRecordOffset + 42);
				std::string EntryFileName =
					std::string(reinterpret_cast<const char*>(FileChunk.Data()) + CurrentRecordOffset + 46, FileNameLength);
				// Zip64 files will use 0xffffffff in the (un)compressed part to direct the parser
				// to the "extra field" section.
				if (CompressedSize == Constants::ZipTag::MAX32 || UncompressedSize == Constants::ZipTag::MAX32 ||
					LocalHeaderOffset == Constants::ZipTag::MAX32)
				{
					Modio::Detail::Buffer& FChunk = FileChunk;
					// The extra field should start after the entry file name, however, I found that some
					// implementations (cuf macos cuf cuf) might not comply with that, so it is necessary
					// to find the "header" first, then parse the (un)compressed size
					std::uint64_t ExtraFieldOffset = CurrentRecordOffset + 46 + FileNameLength;
					std::uint16_t HeaderSize = 0;

					for (std::uint64_t i = ExtraFieldOffset; i < FChunk.GetSize(); i += 4)
					{
						HeaderSize = Modio::Detail::TypedBufferRead<std::uint16_t>(FChunk, i + 2);
						std::uint16_t HeaderFind = Modio::Detail::TypedBufferRead<std::uint16_t>(FChunk, i);
						std::uint32_t HeaderSignature = Modio::Detail::TypedBufferRead<std::uint32_t>(FChunk, i);

						if (HeaderSignature == Constants::ZipTag::CentralFileHeaderSignature)
						{
							// This case means that the payload did not have the header signature for the extra field
							// and the header size did not match any of the expected sizes and reading FChunk we
							// encountered another file entry, which is not a valid zip file and should bail because no
							// size can be read from this entry.
							return std::tuple<ArchiveFileImplementation::ArchiveEntry, std::uint64_t,
											  Modio::ErrorCode> {
								ArchiveFileImplementation::ArchiveEntry(), 0ULL,
								Modio::make_error_code(Modio::ArchiveError::InvalidHeader)};
						}

						// When "HeaderFind" matches the "Extended information field header ID" accompanied by a valid
						// size, then at that moment we are able to read the (un)compressed attributes
						if (HeaderFind == Constants::ZipTag::Zip64ExtraFieldSignature &&
							ValidSignatureSize(HeaderSize) == true)
						{
							// After the header, there are two bytes with "Size of the extra field chunk (8, 16, 24 or
							// 28)" that's why those are added here (2 bytes EIFHID + 2 EIFHID)
							ExtraFieldOffset = i + 4;
							break;
						}
					}

					if (HeaderSize == 8)
					{
						// According to the zip64 specs, when value of the LocalHeaderOffset field is 0xFFFFFFFF
						// the size will be in the corresponding 8 byte zip64 extended information extra field.
						LocalHeaderOffset = Modio::Detail::TypedBufferRead<std::uint64_t>(FChunk, ExtraFieldOffset);
					}
					else if (HeaderSize == 16)
					{
						// The extra field contains the information regarding the (un)compressed data of Zip64
						// files. To access it, it requires the 46 bytes of the current central directory file
						// header + the file name + 4 more bytes (Header ID & Size of extra field)
						UncompressedSize = Modio::Detail::TypedBufferRead<std::uint64_t>(FChunk, ExtraFieldOffset);

						// Where ExtraFieldOffset is plus the UncompressedSize length
						CompressedSize = Modio::Detail::TypedBufferRead<std::uint64_t>(FChunk, ExtraFieldOffset + 8);
					}
					else
					{
						UncompressedSize = Modio::Detail::TypedBufferRead<std::uint64_t>(FChunk, ExtraFieldOffset);
						CompressedSize = Modio::Detail::TypedBufferRead<std::uint64_t>(FChunk, ExtraFieldOffset + 8);
						LocalHeaderOffset =
							Modio::Detail::TypedBufferRead<std::uint64_t>(FChunk, ExtraFieldOffset + 16);
					}
				}

				std::uint64_t PlusOffset = CurrentRecordOffset + 46 + FileNameLength + ExtraFieldLength + CommentLength;
				ArchiveFileImplementation::ArchiveEntry Entry = ArchiveFileImplementation::ArchiveEntry {
					static_cast<ArchiveFileImplementation::CompressionMethod>(CompressionMethod), EntryFileName,
					// Set the Entry FileOffset as the 30 bytes of the header plus the name and extra field length
					LocalHeaderOffset, CompressedSize, UncompressedSize, InputCRC,
					((ExternalAttributes & 0x10) == 0x10)};

				return std::tuple<ArchiveFileImplementation::ArchiveEntry, std::uint64_t, Modio::ErrorCode> {Entry,
																											 PlusOffset,
																											 {}};
			}
		}; // ZipStructures
	} // namespace Detail
} // namespace Modio
