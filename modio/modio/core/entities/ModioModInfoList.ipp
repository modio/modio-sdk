/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioModInfoList.h"
#endif

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::ModInfoList& OutModInfoList)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(OutModInfoList));

		Detail::ParseSafe(Json, OutModInfoList.InternalList, "data");
	}
} // namespace Modio