/*
 *  Copyright (C) 2021-2022 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/ModioDefines.h"

#include "modio/core/ModioSplitCompilation.h"
#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/JsonWrapper.h"
#include <vector>

namespace Modio
{
	/// @docpublic
	/// @brief Class representing a list of users
	class UserList : public PagedResult, public List<std::vector, Modio::User>
	{
	public:
		/// @docpublic
		/// @brief Insert ModInfoList to the end of this list
		void Append(const UserList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}

		/// @docpublic
		/// @brief Insert a ModInfo to the end of this list
		void Append(const User& UserData)
		{
			InternalList.push_back(UserData);
		}

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::UserList& OutUserList);
	};

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioUserList.ipp"
#endif
