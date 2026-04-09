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
		std::int32_t PageIndex = 0;
		std::int32_t PageSize = 0;
		std::int32_t PageCount = 0;
		std::int32_t TotalResultCount = 0;
		std::int32_t ResultCount = 0;

		Detail::ParseSafe(Json, ResultCount, "result_count");
		Detail::ParseSafe(Json, PageSize, "result_limit");
		Detail::ParseSafe(Json, TotalResultCount, "result_total");

		std::int32_t ResultOffset = 0;
		ResultOffset = std::move(0);
		Detail::ParseSafe(Json, ResultOffset, "result_offset");
		PageIndex =
			PageSize ? std::int32_t(std::floor(float(ResultOffset) / float(PageSize))) : 0;
		PageCount =
			PageSize
				? std::int32_t(std::ceil(float(TotalResultCount) / float(PageSize)))
				: 0;

		InitializePageResult(PagedResult, PageIndex, PageSize, PageCount, TotalResultCount, ResultCount);
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