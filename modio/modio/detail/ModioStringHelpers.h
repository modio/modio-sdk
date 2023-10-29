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
#include "modio/detail/HedleyWrapper.h"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
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
			inline bool StartWith(const std::string& String, const std::string& Prefix)
			{
				if (String.length() >= Prefix.length())
				{
					return String.compare(0, Prefix.length(), Prefix) == 0;
				}
				return false;
			}

			inline bool MatchesCaseInsensitive(const std::string& a, const std::string& b)
			{
				return std::equal(a.begin(), a.end(), b.begin(), b.end(),
								  [](unsigned char a, unsigned char b) { return tolower(a) == tolower(b); });
			}

			/// @brief URL-encodes a string
			/// @param String The input to encode
			/// @return The URL-encoded output
			static inline std::string URLEncode(const std::string& String)
			{
				std::string Result;
				Result.reserve(String.size() * 3);
				// @todo: Support UTF8
				auto IsValidCharacter = [](char Character) {
					return (Character >= '0' && Character <= '9') || (Character >= 'a' && Character <= 'z') ||
						   (Character >= 'A' && Character <= 'Z') || Character == '-' || Character == '_' ||
						   Character == '.' || Character == '~';
				};

				static const char HexChars[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
												  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
				for (std::size_t Idx = 0; Idx < String.size(); ++Idx)
				{
					const char& Char = String[Idx];
					if (IsValidCharacter(Char))
					{
						Result += Char;
					}
					else
					{
						Result += '%';
						Result += HexChars[(Char & 0xF0) >> 4];
						Result += HexChars[(Char & 0x0F) >> 0];
					}
				}

				return Result;
			}

			static inline Modio::Optional<std::uint32_t> ParseDateOrInt(std::string Value)
			{
				if (Value == "")
				{
					return {};
				}

				std::uint32_t Seconds = 0;
				std::stringstream IntStream(Value);

				// In case we need to localize it to a certain language/location
				// IntStream.imbue(std::locale("POSIX"));

				// Try to parse the number of seconds first
				IntStream >> Seconds;

				if (Seconds != 0)
				{
					return Seconds;
				}

				std::tm Time = {};
				std::stringstream DateStream(Value);
				// As an example: Fri, 1 Feb 2030 07:28:00 GMT
				DateStream >> std::get_time(&Time, "%a, %d %b %Y %H:%M:%S %Z");
				std::chrono::system_clock::time_point ServerTime =
					std::chrono::system_clock::from_time_t(std::mktime(&Time));
				std::chrono::system_clock::time_point CurrentTime = std::chrono::system_clock::now();
				std::size_t SecondsToWait =
					std::chrono::duration_cast<std::chrono::seconds>(ServerTime - CurrentTime).count();

				// In case debugging of time is necessary
				// std::cout << std::put_time(&Time, "%c") << '\n';

				if (SecondsToWait > 0)
				{
					return static_cast <std::uint32_t>(SecondsToWait);
				}

				return {};
			}

			static std::map<std::string, std::string> ParseRawHeaders(const std::string RawHeaders)
			{
				std::map<std::string, std::string> ResponseHeaders;
				std::string MutHeaders = RawHeaders;
				std::string BreakDelimiter = "\r\n";
				std::string ColonDelimiter = ": ";
				size_t BreakPos = 0;
				size_t ColonPos = 0;
				std::string Line;
				while ((BreakPos = MutHeaders.find(BreakDelimiter)) != std::string::npos)
				{
					Line = MutHeaders.substr(0, BreakPos);
					if ((ColonPos = Line.find(ColonDelimiter)) != std::string::npos)
					{
						std::string Key = Line.substr(0, ColonPos);
						std::string Value = Line.substr(ColonPos + ColonDelimiter.size(), Line.size());
						ResponseHeaders.insert({Key, Value});
					}

					MutHeaders.erase(0, BreakPos + BreakDelimiter.length());
				}

				return ResponseHeaders;
			}

			// These functions create false "unused functions" warnings on certain platforms.
			// Suppressing those warnings here.
			MODIO_DIAGNOSTIC_PUSH
			MODIO_ALLOW_UNUSED_FUNCTIONS

			/// @brief Replaces all occurrences of a string in another string (inline)
			/// @param Str Mutable input string.
			/// @param From The value to replace
			/// @param To The value to substitute in for the From value
			static void ReplaceAll(std::string& Str, const std::string& From, const std::string& To)
			{
				size_t StartPos = 0;
				while ((StartPos = Str.find(From, StartPos)) != std::string::npos)
				{
					Str.replace(StartPos, From.length(), To);
					StartPos += To.length(); // Handles case where 'To' is a substring of 'From'
				}
			}

			/// @brief Splits a string based on a string delimiter
			/// @param Str The string to split
			/// @param Delimiter The delimiter to split the string at
			/// @return Vector containing the substrings after splitting
			static std::vector<std::string> Split(const std::string& Str, const std::string& Delimiter)
			{
				std::vector<std::string> Result;
				size_t OldLoc = 0;
				size_t StartPos = 0;
				while ((StartPos = Str.find(Delimiter, StartPos)) != std::string::npos)
				{
					Result.push_back(Str.substr(OldLoc, StartPos - OldLoc));
					StartPos += Delimiter.length();
					OldLoc = StartPos;
				}
				// in string "cheese beaver donkey", we won't have consumed the donkey here
				// Ensure that we have any more characters to consume
				if (Str.length() - OldLoc > 0)
				{
					Result.push_back(Str.substr(OldLoc, Str.length() - OldLoc));
				}

				return Result;
			}

			/// @brief Checks if a string ends with a specified suffix
			/// @param FullString The string to check
			/// @param Ending The suffix to check against (case-sensitive)
			/// @return True of the string ends with the supplied suffix
			static bool EndsWith(const std::string& FullString, const std::string& Ending)
			{
				if (FullString.length() >= Ending.length())
				{
					return FullString.compare(FullString.length() - Ending.length(), Ending.length(), Ending) == 0;
				}
				return false;
			}

			/// @brief Gets the filename component from a URL
			/// @param URL URL containing a file path
			/// @return Modio::Optional containing the filename if present, or empty Optional if not
			static Modio::Optional<std::string> GetFilenameFromURL(const std::string& URL)
			{
				std::size_t LastPos = URL.find_last_of('/');

				// There was no / in the URL, so can't find out where the filename is
				if (LastPos == std::string::npos)
				{
					return {};
				}

				return URL.substr(LastPos + 1);
			}

			// Re-allow "unused function" warnings
			MODIO_DIAGNOSTIC_POP

		} // namespace String
	} // namespace Detail
} // namespace Modio
