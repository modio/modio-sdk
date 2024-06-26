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

#include "modio/detail/entities/ModioUploadPart.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
		/// @docnone
		inline void from_json(const nlohmann::json& Json, Modio::Detail::UploadSessionPart& Session)
		{
			Detail::ParseSafe(Json, Session.UploadID, "upload_id");
			Detail::ParseSafe(Json, Session.PartNumber, "part_number");
			Detail::ParseSafe(Json, Session.PartSize, "part_size");
			Detail::ParseSafe(Json, Session.DateAdded, "date_added");
		}

		/// @docnone
		inline void to_json(nlohmann::json& Json, const Modio::Detail::UploadSessionPart& Session)
		{
			Json = nlohmann::json {{"upload_id", Session.UploadID},
								   {"part_number", Session.PartNumber},
								   {"part_size", Session.PartSize},
								   {"date_added", Session.DateAdded}};
		}

		/// @docnone
		inline void from_json(const nlohmann::json& Json, Modio::Detail::UploadSessionPartList& List)
		{
			from_json(Json, static_cast<Modio::PagedResult&>(List));
			Modio::Detail::ParseSafe(Json, List.InternalList, "data");
		}

	} // namespace Detail
} // namespace Modio