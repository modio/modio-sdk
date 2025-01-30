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
#include <uuid/uuid.h>

namespace Modio
{
	namespace Detail
	{
		namespace PlatformUtilService
		{
			/// @brief Generates a random Version 4, variant 2, GUID
			/// @return the random GUID value
			inline Modio::GuidV4 GuidCreate()
			{
				Modio::GuidV4 Uuid;
				uuid_generate_random(Uuid.Bytes());
				Uuid.ConvertByteOrder();
				return Uuid;
			}

			/// @brief Convert a GUID in binary representation, into a printable string version
			/// @param InGuid the binary GUID to convert
			/// @return the GUID as a text string
			inline std::string GuidToString(const Modio::GuidV4& InGuid)
			{
				char Result[37] = "";
				Modio::GuidV4 Uuid(InGuid);
				Uuid.ConvertByteOrder();
				uuid_unparse_lower(Uuid.Bytes(), Result);
				return Result;
			}

			/// @brief Converts from a GUID string of the form: "8D8AC610-566D-4EF0-9C22-186B2A5ED793" into a 16byte
			/// GUID value
			/// @param InString the GUID to parse
			/// @return the GUID in binary form, or a NULL guid if the parsing failed.
			inline Modio::GuidV4 GuidFromString(const std::string& InString)
			{
				Modio::GuidV4 Uuid;
				if(InString.size() == 36)
				{
					if (uuid_parse(InString.c_str(), Uuid.Bytes()))
					{
						return std::string();
					}
				}
				Uuid.ConvertByteOrder();
				return Uuid;
			}
		} // namespace PlatformUtilService
	} // namespace Detail
} // namespace Modio
