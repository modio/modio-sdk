/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *   
 */

#pragma once
#include "ModioGeneratedVariables.h"
#include <cstdint>
#include <nlohmann/json.hpp>

namespace Modio
{
	
	/// @brief Base class for all types that represent a page from a set of results
	class PagedResult
	{
		

		std::int32_t PageIndex;
		std::int32_t PageSize;
		std::int32_t PageCount;
		std::int32_t TotalResultCount;
		std::int32_t ResultCount;
	public:
		inline std::int32_t GetPageCount() const
		{
			return PageCount;
		}
		
		inline std::int32_t GetPageIndex() const
		{
			return PageIndex;
		}
		inline std::int32_t GetPageSize() const
		{
			return PageSize;
		}

		inline std::int32_t GetTotalResultCount() const
		{
			return TotalResultCount;
		}

		inline std::int32_t GetResultCount() const
		{
			return ResultCount;
		}

		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::PagedResult& PagedResult);
	};

	
} // namespace Modio


#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioPagedResult.ipp"
#endif
