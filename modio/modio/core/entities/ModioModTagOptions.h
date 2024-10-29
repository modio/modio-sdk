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
#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/JsonWrapper.h"
#include <vector>
#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief Localization data for a tag value
	struct ModTagLocalizationData
	{
		/// @docpublic
		/// @brief Localization data for an individual tag value
		 
		/// @brief The original raw unlocalized tag value used by the REST API
		std::string Tag;

		/// @brief Culture code -> Localized tag value string mapping for all configured languages.
		std::map<std::string, std::string> Translations;

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModTagLocalizationData& TagLocalization);
	};

	/// @docpublic
	/// @brief Struct containing pre-localized display strings for a tag group
	struct LocalizedTagCategory
	{
		/// @brief Localized display string for this tag category's name
		std::string GroupName;

		/// @brief Localized display strings for all valid values in this tag category
		std::vector<std::string> Tags;
	};

	/// @docpublic
	/// @brief Metadata about a group of tags that can be used for filtering mods
	struct ModTagInfo
	{
		/// @brief Raw unlocalized tag group name
		std::string TagGroupName;

		/// @brief Valid raw unlocalized tag values this group contains
		std::vector<std::string> TagGroupValues;

		/// @brief Culture code -> localized tag category name mapping for all configured languages
		std::map<std::string, std::string> TagGroupNameLocData;

		/// @brief Localization data for this tag category's values in all configured languages
		std::vector<ModTagLocalizationData> TagGroupValueLocData;

		/// @brief True if multiple tags from the group can be used simultaneously
		bool bAllowMultipleSelection;

		/// @brief True if only visible by admins
		bool bHidden;

		/// @brief True if only editable by admins
		bool bLocked;

	private:

		/// @brief the culture code for the mod.io locale at the time the mod.io API returned this data
		std::string Locale;

	public:

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModTagInfo& TagInfo);

		/// @brief Returns Localized display string for this tag category and its values in the current mod.io locale set at the time this data was requested from the API
		///	@returns Struct of Tag Category and values.
		MODIO_IMPL Modio::LocalizedTagCategory GetLocalizedTags() const
		{
			LocalizedTagCategory Out;
			Out.GroupName = TagGroupName;
			if (TagGroupValueLocData.empty() == false)
			{
				for (size_t i = 0; i < TagGroupValueLocData.size(); i++)
				{
					for (auto const& [key, value] : TagGroupValueLocData[i].Translations)
					{
						if (key == Locale)
						{
							Out.Tags.push_back(value);
						}
					}
				}
			}
			else
			{
				Out.Tags = TagGroupValues;
			}
			return Out;
		}
	};

	/// @docpublic
	/// @brief Container for a collection of ModTagInfo objects
	class ModTagOptions : public PagedResult, public List<std::vector, ModTagInfo>
	{
		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModTagOptions& Options);
	};
} // namespace Modio
