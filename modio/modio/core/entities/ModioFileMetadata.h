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
		Modio::FileMetadataID MetadataId = Modio::FileMetadataID(0); // @todo: Reiterate this name
		///@brief Unique mod id.
		Modio::ModID ModId = Modio::ModID(-1);
		///@brief Unix timestamp of date file was added.
		std::int64_t DateAdded = 0;
		///@brief Current virus scan status of the file. For newly added files that have yet to be scanned this field
		/// will change frequently until a scan is complete
		VirusScanStatus CurrentVirusScanStatus = Modio::FileMetadata::VirusScanStatus::NotScanned;
		///@brief Was a virus detected?
		VirusStatus CurrentVirusStatus = Modio::FileMetadata::VirusStatus::NoThreat;
		///@brief Size of the file in bytes.
		std::int64_t Filesize = 0;
		///@brief Filename including extension.
		std::string Filename = "";
		///@brief Release version this file represents.
		std::string Version = "";
		///@brief Changelog for the file.
		std::string Changelog = "";
		///@brief Metadata stored by the game developer for this file.
		std::string MetadataBlob = "";
		/// @brief Modfile download URL
		std::string DownloadBinaryURL = "";
		/// @brief Modfile download expiry date
		std::int64_t DownloadExpiryDate = 0;

		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::FileMetadata& FileMetadata);
		friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::FileMetadata& FileMetadata);

		friend bool operator==(const Modio::FileMetadata& A, const Modio::FileMetadata& B)
		{
			if ((A.MetadataId == B.MetadataId) && (A.ModId == B.ModId) && (A.DateAdded == B.DateAdded) &&
				(A.CurrentVirusScanStatus == B.CurrentVirusScanStatus) &&
				(A.CurrentVirusStatus == B.CurrentVirusStatus) && (A.Filesize == B.Filesize) &&
				(A.Filename == B.Filename) && (A.Version == B.Version) && (A.Changelog == B.Changelog) &&
				(A.MetadataBlob == B.MetadataBlob) && (A.DownloadBinaryURL == B.DownloadBinaryURL) &&
				(A.DownloadExpiryDate == B.DownloadExpiryDate))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioFileMetadata.ipp"
#endif