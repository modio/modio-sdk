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
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/AsioWrapper.h"
#include <cstdint>
#include <vector>

namespace Modio
{
	namespace Detail
	{
		class ArchiveFileImplementation
		{
		public:
			enum class CompressionMethod : uint16_t
			{
				Store = 0,
				Deflate = 8
			};

			struct ArchiveEntry
			{
				CompressionMethod Compression = CompressionMethod::Deflate;
				Modio::filesystem::path FilePath;
				std::uintmax_t FileOffset = 0;
				std::uintmax_t CompressedSize = 0;
				std::uintmax_t UncompressedSize = 0;
				std::uint32_t CRCValue = 0;
				bool bIsDirectory = false;
			};

		private:
			std::vector<ArchiveEntry> ArchiveEntries;

		public:
			MODIO_IMPL void AddEntry(std::string FileName, std::uintmax_t FileOffset, std::uintmax_t CompressedSize, std::uintmax_t UncompressedSize,
						  CompressionMethod Compression, std::uint32_t CRCValue, bool bIsDirectory = false);
			MODIO_IMPL void AddEntry(ArchiveEntry Entry);

			/// @brief Path to the underlying archive file
			Modio::filesystem::path FilePath;
			
			std::uintmax_t ZipMagicOffset;
			std::uint64_t NumberOfRecords;
			std::uint64_t CentralDirectorySize;
			std::uint64_t CentralDirectoryOffset;
			Modio::FileSize TotalExtractedSize;
            bool bIsZip64 = false;
            
			MODIO_IMPL std::uintmax_t GetNumberOfEntries();

			MODIO_IMPL std::vector<ArchiveEntry>::iterator begin();

			MODIO_IMPL std::vector<ArchiveEntry>::iterator end();

			//todo: @modio-core possibly add a File member 
		};
	}
}

#ifndef MODIO_SEPARATE_COMPILATION
#include "ArchiveFileImplementation.ipp"
#endif
