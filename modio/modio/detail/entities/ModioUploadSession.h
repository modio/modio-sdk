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
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/entities/ModioAvatar.h"
#include <string>

namespace Modio
{
	namespace Detail
	{
		/// @docpublic
		/// @brief Object representing an upload session for multipart mod file upload
		struct UploadSession
		{
			enum class Status : std::uint8_t
			{
				Incomplete = 0,
				Pending = 1,
				Processing = 2,
				Completed = 3,
				Cancelled = 4
			};

			/// @brief Multipart upload session string
			Modio::Optional<std::string> UploadID = {};

			/// @brief Last reported status of the upload session
			Status UploadStatus = Status::Incomplete;

			friend bool operator==(const UploadSession& A, const UploadSession& B)
			{
				return (A.UploadID == B.UploadID);
			}
		};

		inline void from_json(const nlohmann::json& Json, Modio::Detail::UploadSession& Session)
		{
			Detail::ParseSafe(Json, Session.UploadID, "upload_id");
			Detail::ParseSafe(Json, Session.UploadStatus, "status");
		}

		inline void to_json(nlohmann::json& Json, const Modio::Detail::UploadSession& Session)
		{
			Json = nlohmann::json {{"upload_id", Session.UploadID.value_or("")}, {"status", Session.UploadStatus}};
		}
	} // namespace Detail
} // namespace Modio
