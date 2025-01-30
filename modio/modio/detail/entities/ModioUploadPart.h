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
#include "modio/core/entities/ModioPagedResult.h"
#include <string>

namespace Modio
{
	namespace Detail
	{
		/// @docpublic
		/// @brief Object representing one part of a whole multipart upload session
		struct UploadSessionPart
		{
			/// @brief Related multipart upload session string
			std::string UploadID;

			/// @brief The part number within an upload session
			std::uint32_t PartNumber = 0;

			/// @brief The part size in bytes within an upload session
			std::uint32_t PartSize = 0;

			/// @brief The date added within an upload session
			std::uint64_t DateAdded = 0;

			friend bool operator==(const UploadSessionPart& A, const UploadSessionPart& B)
			{
				return (A.UploadID == B.UploadID) && (A.PartNumber == B.PartNumber) && (A.PartSize == B.PartSize) &&
					   (A.DateAdded == B.DateAdded);
			}

			/// @docnone
			MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::Detail::UploadSessionPart& Session);

			/// @docnone
			MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::Detail::UploadSessionPart& Session);
		};

		/// @docpublic
		/// @brief Container for a collection of UploadSessionPart objects
		class UploadSessionPartList : public PagedResult, public List<std::vector, UploadSessionPart>
		{
			/// @docnone
			MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::Detail::UploadSessionPartList& List);
		};

	} // namespace Detail
} // namespace Modio
