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
	auto format(const Modio::ModID& Integer, FormatCtx& ctx) const
	{
		return fmt::formatter<std::int64_t>::format(Integer, ctx);
	}
};

template<>
struct fmt::formatter<Modio::GameID> : fmt::formatter<std::int64_t>
{
	template<typename FormatCtx>
	auto format(const Modio::GameID& Integer, FormatCtx& ctx) const
	{
		return fmt::formatter<std::int64_t>::format(Integer, ctx);
	}
};

template<>
struct fmt::formatter<Modio::UserID> : fmt::formatter<std::int64_t>
{
	template<typename FormatCtx>
	auto format(const Modio::UserID& Integer, FormatCtx& ctx) const
	{
		return fmt::formatter<std::int64_t>::format(Integer, ctx);
	}
};

template<>
struct fmt::formatter<Modio::FileSize> : fmt::formatter<std::int64_t>
{
	template<typename FormatCtx>
	auto format(const Modio::FileSize& Integer, FormatCtx& ctx) const
	{
		return fmt::formatter<std::int64_t>::format(Integer, ctx);
	}
};


template<>
struct fmt::formatter<Modio::ModCreationHandle> : fmt::formatter<std::int64_t>
{
	template<typename FormatCtx>
	auto format(const Modio::ModCreationHandle& Integer, FormatCtx& ctx) const
	{
		return fmt::formatter<std::int64_t>::format(Integer, ctx);
	}
};

template<>
struct fmt::formatter<Modio::FileOffset> : fmt::formatter<std::int64_t>
{
	template<typename FormatCtx>
	auto format(const Modio::FileOffset& Integer, FormatCtx& ctx) const
	{
		return fmt::formatter<std::int64_t>::format(Integer, ctx);
	}
};