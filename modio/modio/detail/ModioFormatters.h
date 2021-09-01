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
#include "modio/detail/FmtWrapper.h"
template<>
struct fmt::formatter<Modio::ModID> : fmt::formatter<std::int64_t>
{
	template<typename FormatCtx>
	auto format(const Modio::ModID& Integer, FormatCtx& ctx)
	{
		return fmt::formatter<std::int64_t>::format(Integer, ctx);
	}
};