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
#include <nlohmann/json.hpp>
#include <vector>

namespace Modio
{
	/// @docpublic
	/// @brief Metadata about a group of tags that can be used for filtering mods 
	struct ModTagInfo
	{
		/// @brief The display name for the tag
		std::string TagGroupName;
		
		/// @brief The valid tags the group can have
		std::vector<std::string> TagGroupValues;

		/// @brief True if multiple tags from the group can be used simultaneously
		bool bAllowMultipleSelection;

		friend void from_json(const nlohmann::json& Json, Modio::ModTagInfo& TagInfo)
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
	};

	/// @docpublic   
	/// @brief Container for a collection of ModTagInfo objects
	class ModTagOptions : public PagedResult, public List<std::vector, ModTagInfo>
	{
		friend void from_json(const nlohmann::json& Json, Modio::ModTagOptions& Options)
		{
			from_json(Json, static_cast<Modio::PagedResult&>(Options));
			Modio::Detail::ParseSafe(Json, Options.InternalList, "data");
		}
	};
} // namespace Modio