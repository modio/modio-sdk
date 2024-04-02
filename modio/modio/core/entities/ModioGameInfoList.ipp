/*
 *  Copyright (C) 2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioGameInfoList.h"
#endif

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::GameInfoList& OutGameInfoList)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(OutGameInfoList));

		Detail::ParseSafe(Json, OutGameInfoList.InternalList, "data");
	}
} // namespace Modio
