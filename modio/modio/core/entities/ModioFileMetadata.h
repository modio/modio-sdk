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
#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief Metadata for a release archive for a mod
	struct FileMetadata
	{
		enum class VirusScanStatus : std::uint8_t
		{
			NotScanned = 0,
			ScanComplete = 1,
			InProgress = 2,
			TooLargeToScan = 3,
			FileNotFound = 4,
			ErrorScanning = 5
		};

		enum class VirusStatus : std::uint8_t
		{
			NoThreat = 0,
			Malicious = 1
		};

		///@brief Unique modfile id.
		Modio::FileMetadataID MetadataId; // @todo: Reiterate this name
		///@brief Unique mod id.
		Modio::ModID ModId;
		///@brief Unix timestamp of date file was added.
		std::int64_t DateAdded;
		///@brief Current virus scan status of the file. For newly added files that have yet to be scanned this field
		/// will change frequently until a scan is complete
		VirusScanStatus CurrentVirusScanStatus;
		///@brief Was a virus detected?
		VirusStatus CurrentVirusStatus;
		///@brief Size of the file in bytes.
		std::int64_t Filesize;
		///@brief Filename including extension.
		std::string Filename;
		///@brief Release version this file represents.
		std::string Version;
		///@brief Changelog for the file.
		std::string Changelog;
		///@brief Metadata stored by the game developer for this file.
		std::string MetadataBlob;

		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::FileMetadata& FileMetadata);
		friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::FileMetadata& FileMetadata);
	};

	
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioFileMetadata.ipp"
#endif