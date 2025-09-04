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
#include "modio/core/ModioCoreTypes.h"
#if __cplusplus >= 201703L
#include <string_view>
#endif
#include <string>

namespace Modio
{
	namespace Detail
	{
		namespace PlatformUtilService
		{
			/// @brief Generates a random Version 4, variant 2, GUID
			/// @return the random GUID value
			Modio::GuidV4 GuidCreate()
			{
				std::random_device Rd;
				std::mt19937_64 Engine(Rd());
				std::uniform_int_distribution<uint64_t> Rand;
				union
				{
					uint64_t Values64[2];
					uint8_t Values8[16];
				};
				Values64[0] = Rand(Engine);
				Values64[1] = Rand(Engine);
				// manipulate the bits to indicate GUID version 4, variant 2. (Microsoft format)
				Values64[0] = (Values64[0] & 0x0FFFFFFFFFFFFFFFULL) | 0x4000000000000000ULL;
				Values64[1] = (Values64[1] & 0xFFFFFFFFFFFFFF3FULL) | 0x0000000000000080ULL;
				return Modio::GuidV4(Values8);
			}

			/// @brief Convert a GUID in binary representation, into a printable string version
			/// @param InGuid the binary GUID to convert
			/// @return the GUID as a text string
			std::string GuidToString(const Modio::GuidV4& InGuid)
			{
				char Result[36];
				auto ByteToHex = [](uint8_t v, char* out) {
					const char* Hex = "0123456789abcdef";
					out[0] = Hex[(v >> 4)];
					out[1] = Hex[(v & 0xF)];
				};
				Result[23] = Result[18] = Result[13] = Result[8] = '-';
				ByteToHex(InGuid.Bytes()[3], Result);
				ByteToHex(InGuid.Bytes()[2], Result + 2);
				ByteToHex(InGuid.Bytes()[1], Result + 4);
				ByteToHex(InGuid.Bytes()[0], Result + 6);
				ByteToHex(InGuid.Bytes()[5], Result + 9);
				ByteToHex(InGuid.Bytes()[4], Result + 11);
				ByteToHex(InGuid.Bytes()[7], Result + 14);
				ByteToHex(InGuid.Bytes()[6], Result + 16);
				ByteToHex(InGuid.Bytes()[8], Result + 19);
				ByteToHex(InGuid.Bytes()[9], Result + 21);
				ByteToHex(InGuid.Bytes()[10], Result + 24);
				ByteToHex(InGuid.Bytes()[11], Result + 26);
				ByteToHex(InGuid.Bytes()[12], Result + 28);
				ByteToHex(InGuid.Bytes()[13], Result + 30);
				ByteToHex(InGuid.Bytes()[14], Result + 32);
				ByteToHex(InGuid.Bytes()[15], Result + 34);
				return std::string(Result, 36);
			}

			/// @brief Converts from a GUID string of the form: "8D8AC610-566D-4EF0-9C22-186B2A5ED793" into a 16byte
			/// GUID value
			/// @param InString the GUID to parse
			/// @return the GUID in binary form, or a NULL guid if the parsing failed.
			Modio::GuidV4 GuidFromString(const std::string& InString)
			{
				Modio::GuidV4 Uuid;
				if (InString.size() != 36)
				{
					// incorrect string length
					return Uuid;
				}

				const char* Text = InString.c_str();
				if (Text[8] != '-' || Text[13] != '-' || Text[18] != '-' || Text[23] != '-')
				{
					// expected '-' in 4 places
					return Uuid;
				}

				// convert a two byte asci hex number into the numerical representation
				auto TextToByte = [](const char* Text, uint8_t& Byte) {
					char HiChar = char(std::tolower(Text[0]));
					char LoChar = char(std::tolower(Text[1]));
					uint8_t Hi, Lo;

					if (HiChar >= '0' && HiChar <= '9')
						Hi = uint8_t(HiChar - '0');
					else if (HiChar >= 'a' && HiChar <= 'f')
						Hi = uint8_t((HiChar - 'a') + 10);
					else
						return false;

					if (LoChar >= '0' && LoChar <= '9')
						Lo = uint8_t(LoChar - '0');
					else if (LoChar >= 'a' && LoChar <= 'f')
						Lo = uint8_t((LoChar - 'a') + 10);
					else
						return false;

					Byte = uint8_t(Lo | (Hi << 4));
					return true;
				};

				bool Valid = TextToByte(Text + 0, Uuid.Bytes()[3]);
				Valid = Valid && TextToByte(Text + 2, Uuid.Bytes()[2]);
				Valid = Valid && TextToByte(Text + 4, Uuid.Bytes()[1]);
				Valid = Valid && TextToByte(Text + 6, Uuid.Bytes()[0]);
				Valid = Valid && TextToByte(Text + 9, Uuid.Bytes()[5]);
				Valid = Valid && TextToByte(Text + 11, Uuid.Bytes()[4]);
				Valid = Valid && TextToByte(Text + 14, Uuid.Bytes()[7]);
				Valid = Valid && TextToByte(Text + 16, Uuid.Bytes()[6]);
				Valid = Valid && TextToByte(Text + 19, Uuid.Bytes()[8]);
				Valid = Valid && TextToByte(Text + 21, Uuid.Bytes()[9]);
				Valid = Valid && TextToByte(Text + 24, Uuid.Bytes()[10]);
				Valid = Valid && TextToByte(Text + 26, Uuid.Bytes()[11]);
				Valid = Valid && TextToByte(Text + 28, Uuid.Bytes()[12]);
				Valid = Valid && TextToByte(Text + 30, Uuid.Bytes()[13]);
				Valid = Valid && TextToByte(Text + 32, Uuid.Bytes()[14]);
				Valid = Valid && TextToByte(Text + 34, Uuid.Bytes()[15]);
				return Valid ? Uuid : Modio::GuidV4();
			}
		} // namespace PlatformUtilService
	} // namespace Detail

	MODIO_IMPL std::string GuidV4::ToString() const
	{
		return Modio::Detail::PlatformUtilService::GuidToString(*this);
	}

	MODIO_IMPL void GuidV4::FromString(const std::string& String)
	{
		*this = Modio::Detail::PlatformUtilService::GuidFromString(String);
	}

	MODIO_IMPL void GuidV4::Generate()
	{
		*this = Modio::Detail::PlatformUtilService::GuidCreate();
	}

	MODIO_IMPL Guid Guid::GenerateGuid()
	{
		return Guid(Detail::PlatformUtilService::GuidCreate());
	}

	MODIO_IMPL Guid::Guid(const Modio::GuidV4& InGuid)
	{
		InternalGuid = InGuid.ToString();
	}

} // namespace Modio
