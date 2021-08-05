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