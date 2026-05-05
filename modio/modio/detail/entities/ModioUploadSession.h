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
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioAvatar.h"

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
			Modio::Optional<std::string> UploadID {};

			/// @brief Last reported status of the upload session
			Status UploadStatus = Status::Incomplete;

			friend bool operator==(const Modio::Detail::UploadSession& A, const Modio::Detail::UploadSession& B)
			{
				return (A.UploadID == B.UploadID);
			}
		};

		/// @docpublic
		/// @brief Container for a collection of UploadSession objects
		class UploadSessionList : public Modio::PagedResult,
								  public Modio::List<std::vector, Modio::Detail::UploadSession>
		{
		};

	} // namespace Detail
} // namespace Modio
