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

#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/core/entities/ModioUser.h"

namespace Modio
{
	/// @docpublic
	/// @brief Class representing a list of users
	class UserList : public Modio::PagedResult, public Modio::List<std::vector, Modio::User>
	{
	public:
		/// @docpublic
		/// @brief Insert UserList to the end of this list
		void Append(const Modio::UserList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}

		/// @docpublic
		/// @brief Insert a User to the end of this list
		void Append(const Modio::User& UserData)
		{
			InternalList.push_back(UserData);
		}
	};

} // namespace Modio
