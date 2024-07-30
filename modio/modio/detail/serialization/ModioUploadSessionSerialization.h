/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/entities/ModioUploadSession.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
		
		inline void from_json(const nlohmann::json& Json, Modio::Detail::UploadSession& Session)
		{
			std::string UploadID;
			if (Detail::ParseSafe(Json, UploadID, "upload_id"))
			{
				Session.UploadID = UploadID;
			}
			Detail::ParseSafe(Json, Session.UploadStatus, "status");
		}

		inline void to_json(nlohmann::json& Json, const Modio::Detail::UploadSession& Session)
		{
			Json = nlohmann::json {{"upload_id", Session.UploadID.value_or("")}, {"status", Session.UploadStatus}};
		}

		inline void from_json(const nlohmann::json& Json, Modio::Detail::UploadSessionList& List)
		{
			from_json(Json, static_cast<Modio::PagedResult&>(List));
			Modio::Detail::ParseSafe(Json, List.InternalList, "data");
		}

	} // namespace Detail
} // namespace Modio