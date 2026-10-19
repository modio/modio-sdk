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

#include "modio/core/entities/ModioModFileCookInfo.h"
#include "modio/detail/JsonWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/serialization/ModioFileMetadataSerialization.h"
#include "modio/detail/serialization/ModioStrongIntegerSerialization.h"

namespace Modio
{
	struct ModFileCookInfo;

	NLOHMANN_JSON_SERIALIZE_ENUM(Modio::ModFileCookStatus, {{Modio::ModFileCookStatus::Queued, 0},
													 {Modio::ModFileCookStatus::Cooking, 1},
													 {Modio::ModFileCookStatus::Successful, 2},
													 {Modio::ModFileCookStatus::Failed, 3},
													 {Modio::ModFileCookStatus::ModerationPending, 4},
													 {Modio::ModFileCookStatus::ModerationRejected, 5}});

	inline void from_json(const nlohmann::json& Json, ModFileCookLog& Entry)
	{
		Detail::ParseSafe(Json, Entry.IsPrivate, "is_private");
		Detail::ParseSafe(Json, Entry.Level, "level");
		Detail::ParseSafe(Json, Entry.Message, "message");
	}

	inline void to_json(nlohmann::json& Json, const ModFileCookLog& Entry)
	{
		Json = nlohmann::json {{"is_private", Entry.IsPrivate}, {"level", Entry.Level}, {"message", Entry.Message}};
	}

	inline void from_json(const nlohmann::json& Json, ModFileCookMetadata& Metadata)
	{
		Detail::ParseSafe(Json, Metadata.CookTimeSeconds, "cook_time_seconds");
		Detail::ParseSafe(Json, Metadata.DownloadTimeSeconds, "download_time_seconds");
		Detail::ParseSafe(Json, Metadata.FileCompressionTimeSeconds, "file_compression_time_seconds");
		Detail::ParseSafe(Json, Metadata.FileDecompressionTimeSeconds, "file_decompression_time_seconds");
		Detail::ParseSafe(Json, Metadata.TotalJobTimeSeconds, "total_job_time_seconds");
		Detail::ParseSafe(Json, Metadata.UploadTimeSeconds, "upload_time_seconds");
	}

	inline void to_json(nlohmann::json& Json, const ModFileCookMetadata& Metadata)
	{
		Json = nlohmann::json {{"cook_time_seconds", Metadata.CookTimeSeconds},
							   {"download_time_seconds", Metadata.DownloadTimeSeconds},
							   {"file_compression_time_seconds", Metadata.FileCompressionTimeSeconds},
							   {"file_decompression_time_seconds", Metadata.FileDecompressionTimeSeconds},
							   {"total_job_time_seconds", Metadata.TotalJobTimeSeconds},
							   {"upload_time_seconds", Metadata.UploadTimeSeconds}};
	}

	/// @docnone
	inline void from_json(const nlohmann::json& Json, Modio::ModFileCookInfo& ModFileCook)
	{
		std::string debug = Json.dump();

		Detail::ParseSafe(Json, ModFileCook.CookUuid, "cook_uuid");
		Detail::ParseSafe(Json, ModFileCook.Modfile, "modfile");
		Detail::ParseSafe(Json, ModFileCook.Platform, "platform");
		Detail::ParseSafe(Json, ModFileCook.Status, "status");
		Detail::ParseSafe(Json, ModFileCook.DateAdded, "date_added");
		Detail::ParseSafe(Json, ModFileCook.DateUpdated, "date_updated");
		Detail::ParseSafe(Json, ModFileCook.Metadata, "metadata");
		Detail::ParseSafe(Json, ModFileCook.Logs, "logs");
		Detail::ParseSafe(Json, ModFileCook.Filename, "filename");
		Detail::ParseSafe(Json, ModFileCook.Filesize, "filesize");
		Detail::ParseSafe(Json, ModFileCook.Version, "version");
	}

	/// @docnone
	inline void to_json(nlohmann::json& Json, const Modio::ModFileCookInfo& ModFileCook)
	{
		Json = nlohmann::json {{"cook_uuid", ModFileCook.CookUuid},
							   {"modfile", ModFileCook.Modfile},
							   {"platform", ModFileCook.Platform},
							   {"status", ModFileCook.Status},
							   {"date_added", ModFileCook.DateAdded},
							   {"date_updated", ModFileCook.DateUpdated},
							   {"metadata", ModFileCook.Metadata},
							   {"logs", ModFileCook.Logs},
							   {"filename", ModFileCook.Filename},
							   {"filesize", ModFileCook.Filesize},
							   {"version", ModFileCook.Version}};
	}

} // namespace Modio
