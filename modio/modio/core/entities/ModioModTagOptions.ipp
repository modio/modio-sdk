/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *  
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioModTagOptions.h"
#endif

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::ModTagInfo& TagInfo)
	{
		Modio::Detail::ParseSafe(Json, TagInfo.TagGroupValues, "tags");
		Modio::Detail::ParseSafe(Json, TagInfo.TagGroupName, "name");
		if (Json.contains("type") && !Json.at("type").is_null())
		{
			std::string TypeValue = "";
			Json.at("type").get_to<std::string>(TypeValue);
			if (TypeValue.compare("checkboxes"))
			{
				TagInfo.bAllowMultipleSelection = false;
			}
			else
			{
				TagInfo.bAllowMultipleSelection = true;
			}
		}
	}

	void from_json(const nlohmann::json& Json, Modio::ModTagOptions& Options)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(Options));
		Modio::Detail::ParseSafe(Json, Options.InternalList, "data");
	}

}

