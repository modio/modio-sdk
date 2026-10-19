/*
 *  Copyright (C) 2021-2026 mod.io Pty Ltd. <https://mod.io>
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
#include "modio/core/entities/ModioUserRoleInfo.h"

namespace Modio
{
	/// @docpublic
	/// @brief Class representing a list of user roles
	class UserRoleInfoList : public Modio::PagedResult, public Modio::List<std::vector, Modio::UserRoleInfo>
	{
	public:

		/// @docpublic
		/// @brief Insert a UserRoleInfo to the end of this list
		void Append(const Modio::UserRoleInfo& UserData)
		{
			InternalList.push_back(UserData);
		}
	};

} // namespace Modio
