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
#include "ModioGeneratedVariables.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/AsioWrapper.h"
#include <cstdint>
#include <vector>

namespace Modio
{
	namespace Detail
	{
        namespace Constants
        {
            namespace ZipTag
            {
                // Zip64
                constexpr uint32_t EndCentralDirectory64 = 0x06064b50; // End of central directory signature Zip64
                constexpr uint32_t EndCentralDirectoryLocator64   = 0x07064b50; // End of central directory locator
                constexpr uint32_t EndCentralDirectoryHeaderSize64 = 56; // End of central directory header size Zip64
                constexpr uint32_t EndCentralDirectoryLocatorSize64 = 20; // End of central directory locator size
                constexpr uint16_t ExtendedInformationFieldHeaderID = 0x0001; // Extended information field header ID
                constexpr uint32_t DataDescriptorID   = 0x08074b50; // Data descriptor ID
                constexpr uint32_t DataDescriptorSize64 = 24; // Data descriptor size Zip64
                constexpr uint16_t TotalExtraFieldSizeFileEntries  = 28; // Total Extra Field Size used in File Entries
                constexpr uint16_t ExtraFieldSize   = 24; // Extra Field Size with (un)compressed and offset
                constexpr uint16_t Zip64Version = 45; // Min version required for Zip64 decompression
            
                // Zip
                constexpr static uint32_t EndCentralDirectory   = 0x06054b50; // End of central directory signature
                constexpr static uint32_t CentralDirectoryFileHeader   = 0x02014b50; // Central directory file header signature
                constexpr static uint32_t OptionalDataDescriptor   = 0x08074b50; // Optional data descriptor signature
                constexpr static uint32_t LocalDirectoryHeader   = 0x04034b50; // Local directory header signature
                constexpr static uint32_t LocalDirectoryHeaderSize  = 30; // Local directory header size
                constexpr static uint32_t CentralDirectoryHeaderSize  = 30; // Central directory header size
                constexpr static uint32_t EndCentralDirectoryHeaderSize = 22; // End of central directory header size
                constexpr static uint32_t DataDescriptorSize   = 16; // Data descriptor size
                constexpr static uint16_t ZipVersion   = 20; // Min version required for Zip64 decompression
            
                constexpr static uint32_t MAX32 = 0xffffffff; // Signal used to denote out-of-bounds
                constexpr static uint16_t MAX16 = 0xffff; // Signal used to denote out-of-bounds
                constexpr static uint16_t Deflate  = 8; // Compression with DEFLATE
            }
        }
    
        struct ZipStructures
        {
            static bool ValidSignatureSize(const uint16_t& Size)
            {
                return (Size == 8) || (Size == 16) || (Size == 24) || (Size == 28);
            }
            
            static std::pair<std::uintmax_t, bool> FindOffsetInBuffer(Modio::Detail::Buffer& Chunk, bool PreZip64)
            {
                std::uintmax_t LocalPosition = Chunk.GetSize() - 4;
                bool IsZip64 = false;
                std::uintmax_t MagicOffset = 0;
                
                for (; LocalPosition > 0; LocalPosition--)
                {
                    uint32_t Value =
                        Modio::Detail::TypedBufferRead<std::uint32_t>(Chunk, LocalPosition);
                    // Prioritize reading the Zip64 signature, which contains accurate information
                    // with large files.
                    if (Value == Constants::ZipTag::EndCentralDirectory64)
                    {
                        IsZip64 = true;
                        MagicOffset = LocalPosition;
                        break;
                    }
                    // When a file is larger than UINT32_MAX, it is expected to be a Zip64.
                    // Here we force the parser to find the ECDS64 instead of reading the
                    // base ECDS, which is included in all zip files regardless
                    else if (Value == Constants::ZipTag::EndCentralDirectory && PreZip64 == false)
                    {
                        MagicOffset = LocalPosition;
                        break;
                    }
                }
                
                return {MagicOffset, IsZip64};
            }
            
            static std::tuple<uint64_t, uint64_t, uint64_t> ReadCentralDirectory(Modio::Detail::Buffer& Chunk, bool IsZip64)
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
                
                return {NumberOfRecords, CentralDirectorySize, CentralDirectoryOffset};
            }
            
            static std::tuple<ArchiveFileImplementation::ArchiveEntry, std::uint64_t, Modio::ErrorCode> ArchiveParse(Modio::Detail::Buffer& FileChunk, std::uint64_t CurrentRecordOffset)
            {
                std::uint16_t CompressionMethod = Modio::Detail::TypedBufferRead<std::uint16_t>(
                    FileChunk, CurrentRecordOffset + 10);
                std::uint32_t InputCRC = Modio::Detail::TypedBufferRead<std::uint32_t>(
                    FileChunk, CurrentRecordOffset + 16);
                std::uint64_t CompressedSize = Modio::Detail::TypedBufferRead<std::uint32_t>(
                    FileChunk, CurrentRecordOffset + 20);
                std::uint64_t UncompressedSize = Modio::Detail::TypedBufferRead<std::uint32_t>(
                    FileChunk, CurrentRecordOffset + 24);
                std::uint16_t FileNameLength = Modio::Detail::TypedBufferRead<std::uint16_t>(
                    FileChunk, CurrentRecordOffset + 28);
                std::uint16_t ExtraFieldLength = Modio::Detail::TypedBufferRead<std::uint16_t>(
                    FileChunk, CurrentRecordOffset + 30);
                std::uint16_t CommentLength = Modio::Detail::TypedBufferRead<std::uint16_t>(
                    FileChunk, CurrentRecordOffset + 32);
                std::uint32_t ExternalAttributes = Modio::Detail::TypedBufferRead<std::uint32_t>(
                    FileChunk, CurrentRecordOffset + 36);
                std::uint64_t LocalHeaderOffset = Modio::Detail::TypedBufferRead<std::uint32_t>(
                    FileChunk, CurrentRecordOffset + 42);
                std::string EntryFileName = std::string((const char*)FileChunk.Data() + CurrentRecordOffset + 46, FileNameLength);
                // Zip64 files will use 0xffffffff in the (un)compressed part to direct the parser
                // to the "extra field" section.
                if (CompressedSize == Constants::ZipTag::MAX32 ||
                    UncompressedSize == Constants::ZipTag::MAX32 ||
                    LocalHeaderOffset == Constants::ZipTag::MAX32)
                {
                    Modio::Detail::Buffer &FChunk = FileChunk;
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

                        if (HeaderSignature == Constants::ZipTag::CentralDirectoryFileHeader)
                        {
                            // This case means that the payload did not have the header signature for the extra field
                            // and the header size did not match any of the expected sizes and reading FChunk we encountered
                            // another file entry, which is not a valid zip file and should bail because no size can be
                            // read from this entry.
                            return{ArchiveFileImplementation::ArchiveEntry(), 0, Modio::make_error_code(Modio::ArchiveError::InvalidHeader)};
                        }

                        // When "HeaderFind" matches the "Extended information field header ID" accompanied by a valid size,
                        // then at that moment we are able to read the (un)compressed attributes
                        if (HeaderFind == Constants::ZipTag::ExtendedInformationFieldHeaderID &&
                            ValidSignatureSize(HeaderSize) == true)
                        {
                            // After the header, there are two bytes with "Size of the extra field chunk (8, 16, 24 or 28)"
                            // that's why those are added here (2 bytes EIFHID + 2 EIFHID)
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
                        LocalHeaderOffset = Modio::Detail::TypedBufferRead<std::uint64_t>(FChunk, ExtraFieldOffset + 16);
                    }
                }
                
                std::uint64_t PlusOffset = CurrentRecordOffset + 46 + FileNameLength + ExtraFieldLength + CommentLength;
                ArchiveFileImplementation::ArchiveEntry Entry = ArchiveFileImplementation::ArchiveEntry {
                    static_cast<ArchiveFileImplementation::CompressionMethod>(CompressionMethod),
                    EntryFileName,
                    // Set the Entry FileOffset as the 30 bytes of the header plus the name and extra field length
                    LocalHeaderOffset,
                    CompressedSize,
                    UncompressedSize,
                    InputCRC, ((ExternalAttributes & 0x10)== 0x10)};
                
                return {Entry, PlusOffset, {}};
            }
        }; // ZipStructures
    } // Detail namespace
} // Modio namespace
