/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioFileMetadata.h"

#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief Possible status as received from the server
	enum class ModFileCookStatus : std::int32_t
	{
		Queued = 0,
		Cooking = 1,
		Successful = 2,
		Failed = 3,
		ModerationPending = 4,
		ModerationRejected = 5
	};

	/// @docpublic
	/// @brief Possible status as received from the server
	enum class ModFileCookLogSeverity : std::int32_t
	{
		Trace = 0,
		Debug = 1,
		Information = 2,
		Warning = 3,
		Error = 4,
		Critical = 5,
		None = 6
	};

	namespace Detail
	{
		/// @docpublic
		/// @brief Transform a ModFileCookStatus to a std::string
		inline std::string ToString(Modio::ModFileCookStatus CookStatus)
		{
			switch (CookStatus)
			{
				case Modio::ModFileCookStatus::Queued:
					return "Queued";
				case Modio::ModFileCookStatus::Cooking:
					return "Cooking";
				case Modio::ModFileCookStatus::Successful:
					return "Successful";
				case Modio::ModFileCookStatus::Failed:
					return "Failed";
				case Modio::ModFileCookStatus::ModerationPending:
					return "Moderation Pending";
				case Modio::ModFileCookStatus::ModerationRejected:
					return "Moderation Rejected";
			}

			assert(false && "Invalid value to ToString(Modio::ModFileCookStatus)");
			return "Unknown";
		}

		/// @docpublic
		/// @brief Transform a ModFileCookLogSeverity to a std::string
		inline std::string ToString(Modio::ModFileCookLogSeverity CookLogSeverity)
		{
			switch (CookLogSeverity)
			{
				case Modio::ModFileCookLogSeverity::Trace:
					return "Trace";
				case Modio::ModFileCookLogSeverity::Debug:
					return "Debug";
				case Modio::ModFileCookLogSeverity::Information:
					return "Information";
				case Modio::ModFileCookLogSeverity::Warning:
					return "Warning";
				case Modio::ModFileCookLogSeverity::Error:
					return "Error";
				case Modio::ModFileCookLogSeverity::Critical:
					return "Critical";
				case Modio::ModFileCookLogSeverity::None:
					return "None";
			}

			assert(false && "Invalid value to ToString(Modio::ModFileCookLogSeverity)");
			return "Unknown";
		}
	} // namespace Detail

	struct ModFileCookLog
	{
		bool IsPrivate = false;
		ModFileCookLogSeverity Level = ModFileCookLogSeverity::None;
		std::string Message;

		/// @docnone
		friend bool operator==(const Modio::ModFileCookLog& A, const Modio::ModFileCookLog& B)
		{
			if ((A.IsPrivate == B.IsPrivate) && (A.Level == B.Level) && (A.Message == B.Message))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	struct ModFileCookMetadata
	{
		std::int64_t CookTimeSeconds = 0;
		std::int64_t DownloadTimeSeconds = 0;
		std::int64_t FileCompressionTimeSeconds = 0;
		std::int64_t FileDecompressionTimeSeconds = 0;
		std::int64_t TotalJobTimeSeconds = 0;
		std::int64_t UploadTimeSeconds = 0;

		/// @docnone
		friend bool operator==(const Modio::ModFileCookMetadata& A, const Modio::ModFileCookMetadata& B)
		{
			if ((A.CookTimeSeconds == B.CookTimeSeconds) && (A.DownloadTimeSeconds == B.DownloadTimeSeconds) &&
				(A.FileCompressionTimeSeconds == B.FileCompressionTimeSeconds) &&
				(A.FileDecompressionTimeSeconds == B.FileDecompressionTimeSeconds) &&
				(A.TotalJobTimeSeconds == B.TotalJobTimeSeconds) && (A.UploadTimeSeconds == B.UploadTimeSeconds))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	/// @docpublic
	/// @brief Full mod file cook info including the platform it's cooked for, the current status and logs
	struct ModFileCookInfo
	{
		/// @brief The UUID of a cook.
		std::string CookUuid;
		/// @brief Unique modfile id.
		Modio::FileMetadataID Modfile = Modio::FileMetadataID(0);
		/// @brief The platform being cooked for
		std::vector<Modio::ModfilePlatform> Platform {};
		/// @brief The status of the cook.
		Modio::ModFileCookStatus Status = Modio::ModFileCookStatus::Queued;
		/// @brief Unix timestamp the cook was started.
		std::int64_t DateAdded = 0;
		/// @brief Unix timestamp the cook was last updated.
		std::int64_t DateUpdated = 0;
		/// @brief The metadata of the cook.
		Modio::ModFileCookMetadata Metadata {};
		/// @brief The logs of the cook.
		std::vector<Modio::ModFileCookLog> Logs {};
		/// @brief Filename including extension.
		std::string Filename;
		/// @brief Size of the file in bytes.
		std::uint64_t Filesize = 0;
		/// @brief Release version this file represents.
		std::string Version;

		/// @docnone
		friend bool operator==(const Modio::ModFileCookInfo& A, const Modio::ModFileCookInfo& B)
		{
			if ((A.CookUuid == B.CookUuid) && (A.Modfile == B.Modfile) &&
				(A.Platform == B.Platform) && (A.Status == B.Status) &&
				(A.DateAdded == B.DateAdded) && (A.DateUpdated == B.DateUpdated) &&
				(A.Metadata == B.Metadata) && (A.Logs == B.Logs) && (A.Filename == B.Filename) &&
				(A.Filesize == B.Filesize) && (A.Version == B.Version))
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
