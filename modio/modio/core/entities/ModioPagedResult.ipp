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
	#include "modio/core/entities/ModioPagedResult.h"
#endif

#include "modio/detail/JsonWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::PagedResult& PagedResult)
	{
		Detail::ParseSafe(Json, PagedResult.ResultCount, "result_count");
		Detail::ParseSafe(Json, PagedResult.PageSize, "result_limit");
		Detail::ParseSafe(Json, PagedResult.TotalResultCount, "result_total");

		// Convert offset to pages
		int ResultOffset = 0;
		ResultOffset = std::move(0);
		Detail::ParseSafe(Json, ResultOffset, "result_offset");
		PagedResult.PageIndex = PagedResult.PageSize ? (std::int32_t) std::floor(ResultOffset / (float) PagedResult.PageSize) : 0;
		PagedResult.PageCount = PagedResult.PageSize ? (std::int32_t) std::ceil(PagedResult.TotalResultCount / (float) PagedResult.PageSize) : 0;
	};
} // namespace Modio