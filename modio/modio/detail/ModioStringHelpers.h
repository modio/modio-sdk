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

#include "modio/core/ModioStdTypes.h"
#include <map>
#include <vector>

namespace Modio
{
	namespace Detail
	{
		namespace String
		{
			/// @brief Checks if the string starts with the specified prefix
			/// @param String String to check
			/// @param Prefix Prefix to check against
			/// @return true if the string starts with the prefix
			bool StartWith(const std::string& String, const std::string& Prefix);

			bool MatchesCaseInsensitive(const std::string& a, const std::string& b);

			/// @brief URL-encodes a string
			/// @param String The input to encode
			/// @return The URL-encoded output
			std::string URLEncode(const std::string& String);

			Modio::Optional<std::uint32_t> ParseDateOrInt(std::string Value);

			std::map<std::string, std::string> ParseRawHeaders(const std::string RawHeaders);

			/// @brief Replaces all occurrences of a string in another string (inline)
			/// @param Str Mutable input string.
			/// @param From The value to replace
			/// @param To The value to substitute in for the From value
			void ReplaceAll(std::string& Str, const std::string& From, const std::string& To);

			/// @brief Splits a string based on a string delimiter
			/// @param Str The string to split
			/// @param Delimiter The delimiter to split the string at
			/// @return Vector containing the substrings after splitting
			std::vector<std::string> Split(const std::string& Str, const std::string& Delimiter);

			/// @brief Checks if a string ends with a specified suffix
			/// @param FullString The string to check
			/// @param Ending The suffix to check against (case-sensitive)
			/// @return True of the string ends with the supplied suffix
			bool EndsWith(const std::string& FullString, const std::string& Ending);

			/// @brief Gets the filename component from a URL
			/// @param URL URL containing a file path
			/// @return Modio::Optional containing the filename if present, or empty Optional if not
			Modio::Optional<std::string> GetFilenameFromURL(const std::string& URL);

			/// @brief Converts a string to all lowercase
			/// @param str The string to convert to lowercase
			/// @return an all lowercase string
			std::string ToLowercase(const std::string& str);

		} // namespace String
	} // namespace Detail

	/// @brief converts from a string_view to a string
	/// @param S the string view to convert
	/// @return the string
	static inline std::string ToModioString(const std::string_view& S)
	{
		return std::string(S.begin(), S.end());
	}

	/// @brief converts from a string to a string (passthrough)
	/// @param S the string to pass through
	/// @return the string
	static inline const std::string& ToModioString(const std::string& S)
	{
		return S;
	}

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/ModioStringHelpers.ipp"
#endif