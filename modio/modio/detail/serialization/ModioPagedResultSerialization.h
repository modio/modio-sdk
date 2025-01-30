/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioPagedResult.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::PagedResult& PagedResult)
	{
		Detail::ParseSafe(Json, PagedResult.ResultCount, "result_count");
		Detail::ParseSafe(Json, PagedResult.PageSize, "result_limit");
		Detail::ParseSafe(Json, PagedResult.TotalResultCount, "result_total");

		// Convert offset to pages
		int ResultOffset = 0;
		ResultOffset = std::move(0);
		Detail::ParseSafe(Json, ResultOffset, "result_offset");
		PagedResult.PageIndex =
			PagedResult.PageSize ? std::int32_t(std::floor(float(ResultOffset) / float(PagedResult.PageSize))) : 0;
		PagedResult.PageCount =
			PagedResult.PageSize
				? std::int32_t(std::ceil(float(PagedResult.TotalResultCount) / float(PagedResult.PageSize)))
				: 0;
	}

	inline void InitializePageResult(Modio::PagedResult& PagedResult, std::int32_t PageIndex, std::int32_t PageSize,
									 std::int32_t PageCount, std::int32_t TotalResultCount, std::int32_t ResultCount)
	{
		PagedResult.PageIndex = PageIndex;
		PagedResult.PageSize = PageSize;
		PagedResult.PageCount = PageCount;
		PagedResult.TotalResultCount = TotalResultCount;
		PagedResult.ResultCount = ResultCount;
	}
} // namespace Modio