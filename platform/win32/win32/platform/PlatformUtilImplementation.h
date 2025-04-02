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

#pragma once
#include "modio/core/ModioCoreTypes.h"

#pragma comment(lib, "rpcrt4.lib")
#if __cplusplus >= 201703L
#include <string_view>
#endif
#include <string>

#include <Windows.h>
#include <iostream>
#include <rpc.h>
namespace Modio
{
	namespace Detail
	{
		namespace PlatformUtilService
		{
			/// @brief Generates a random Version 4, variant 2, GUID
			/// @return the random GUID value
			inline GuidV4 GuidCreate()
			{
				GuidV4 UU_ID;
				UUID Temp;
				switch (UuidCreate(&Temp))
				{
					case RPC_S_OK:
						std::memcpy(UU_ID.Bytes(), &Temp, 16);
						break;
					case RPC_S_UUID_LOCAL_ONLY:
					case RPC_S_UUID_NO_ADDRESS:
						std::memset(UU_ID.Bytes(), 0, 16);
						return GuidV4();
				}
				return UU_ID;
			}

			/// @brief Convert a GUID in binary representation, into a printable string version
			/// @param InGuid the binary GUID to convert
			/// @return the GUID as a text string
			inline std::string GuidToString(const Modio::GuidV4& InGuid)
			{
				UUID Uuid;
				std::memmove(&Uuid, InGuid.Bytes(), 16);

				RPC_CSTR String = 0;
				if (RPC_S_OUT_OF_MEMORY == UuidToStringA(&Uuid, &String))
				{
					// error
					return std::string();
				}
				std::string Result(reinterpret_cast<const char*>(String), 36);
				RpcStringFreeA(&String);
				return Result;
			}

			/// @brief Converts from a GUID string of the form: "8D8AC610-566D-4EF0-9C22-186B2A5ED793" into a 16byte
			/// GUID value
			/// @param InString the GUID to parse
			/// @return the GUID in binary form, or a NULL guid if the parsing failed.
			inline Modio::GuidV4 GuidFromString(const std::string& InString)
			{
				if (InString.size() != 36)
				{
					// invalid string length
					return Modio::GuidV4();
				}

				UUID Uuid;
				char* StringData = const_cast<char*>(InString.c_str());
				if (RPC_S_INVALID_STRING_UUID == UuidFromStringA(reinterpret_cast<unsigned char*>(StringData), &Uuid))
				{
					// invalid string
					return Modio::GuidV4();
				}

				// Copy into the result. This assumes that the binary representation of our
				// GUIDs matches that of Microsoft.
				Modio::GuidV4 Result(reinterpret_cast<const uint8_t*>(&Uuid));
				if (!Result.IsValid())
				{
					// We should probably assert here!
					// Given that IsValid() only tests for Version 4, Variant 2 UUIDs,
					// we could theoretically get here if we passed in say, a version 2.
					return Modio::GuidV4();
				}
				return Result;
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

	#if __cplusplus >= 202002L
	/// @brief converts from a u8string_view to a string
	/// @param S the u8 string view to convert
	/// @return the string
	static inline std::string ToModioString(const std::u8string_view& S)
	{
		return std::string(S.begin(), S.end());
	}

	/// @brief converts from a u8string to a string
	/// @param S the u8 string to convert
	/// @return the string
	static inline std::string ToModioString(const std::u8string& S)
	{
		return std::string(S.begin(), S.end());
	}
	#endif
} // namespace Modio
