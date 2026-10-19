/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioModFileCookInfoList.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/serialization/ModioPagedResultSerialization.h"
#include "modio/detail/serialization/ModioModFileCookInfoSerialization.h"
namespace Modio
{
	class ModFileCookInfoList;
	class PagedResult;

	inline void from_json(const nlohmann::json& Json, Modio::ModFileCookInfoList& OutModFileCookList)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(OutModFileCookList));

		Detail::ParseSafe(Json, OutModFileCookList.GetRawList(), "data");
	}
} // namespace Modio