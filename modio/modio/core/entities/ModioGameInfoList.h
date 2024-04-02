/*
 *  Copyright (C) 2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "modio/detail/ModioDefines.h"

#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioGameInfo.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/JsonWrapper.h"
#include <vector>

namespace Modio
{
	/// @docpublic
	/// @brief Class representing a list of games that may be a page from a larger set of results
	class GameInfoList : public PagedResult, public List<std::vector, Modio::GameInfo>
	{
	public:
		/// @docpublic
		/// @brief Insert GameInfoList to the end of this list
		void Append(const GameInfoList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}

		/// @docpublic
		/// @brief Insert a GameInfo to the end of this list
		void Append(const GameInfo& GameInfoData)
		{
			InternalList.push_back(GameInfoData);
		}

		/// @docnone
		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::GameInfoList& OutGameInfoList);
	};

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioGameInfoList.ipp"
#endif
