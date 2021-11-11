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

// FNV1a c++11 constexpr compile time hash functions, 32 and 64 bit
// str should be a null terminated string literal, value should be left out
// e.g hash_32_fnv1a_const("example")
// code license: public domain
// post: https://notes.underscorediscovery.com/constexpr-fnv1a/
namespace Modio
{
	namespace Detail
	{
		constexpr uint32_t val_32_const = 0x811c9dc5;
		constexpr uint32_t prime_32_const = 0x1000193;
		constexpr uint64_t val_64_const = 0xcbf29ce484222325;
		constexpr uint64_t prime_64_const = 0x100000001b3;

		inline constexpr uint32_t hash_32_fnv1a_const(const char* const str,
													  const uint32_t value = val_32_const) noexcept
		{
			return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
		}

		inline constexpr uint64_t hash_64_fnv1a_const(const char* const str,
													  const uint64_t value = val_64_const) noexcept
		{
			return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t(str[0])) * prime_64_const);
		}

		inline constexpr std::uint64_t operator ""_hash(const char* str, std::size_t)
		{
			return hash_64_fnv1a_const(str);
		}
	} // namespace Detail
} // namespace Modio
