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
#include "modio/detail/ModioDefines.h"

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/JsonWrapper.h"
#include <vector>

namespace Modio
{
	struct UserRating
	{
		Modio::GameID GameId;
		Modio::ModID ModId;
		Modio::Rating ModRating;

		friend bool operator==(const Modio::UserRating& A, const Modio::UserRating& B) 
		{
			return (A.GameId == B.GameId) && (A.ModId == B.ModId) && (A.ModRating == B.ModRating);
		}
	};

	/// @docpublic
	/// @brief Object representing a mod.io user's ratings
	struct UserRatingList : public PagedResult, public List<std::vector, Modio::UserRating>
	{
		/// @docpublic
		/// @brief Insert ModInfoList to the end of this list
		void Append(const UserRatingList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}

		/// @docpublic
		/// @brief Insert a ModInfo to the end of this list
		void Append(const UserRating& UserRatingData)
		{
			InternalList.push_back(UserRatingData);
		}

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::UserRatingList& Ratings);

	};
} // namespace Modio
