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

#include "modio/core/entities/ModioModTagOptions.h"

#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioSDKSessionData.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::ModTagLocalizationData& TagLocalization)
	{
		Modio::Detail::ParseSafe(Json, TagLocalization.Tag, "tag");
		Modio::Detail::ParseSafe(Json, TagLocalization.Translations, "translations");
	}

	inline void from_json(const nlohmann::json& Json, Modio::ModTagInfo& TagInfo)
	{
		Modio::Detail::ParseSafe(Json, TagInfo.TagGroupValues, "tags");
		Modio::Detail::ParseSafe(Json, TagInfo.TagGroupName, "name");
		Modio::Detail::ParseSafe(Json, TagInfo.TagGroupNameLocData, "name_localization");
		Modio::Detail::ParseSafe(Json, TagInfo.TagGroupValueLocData, "tags_localization");
		if (Json.contains("type") && !Json.at("type").is_null())
		{
			std::string TypeValue;
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
		TagInfo.Locale = Modio::Detail::ToString(Modio::Detail::SDKSessionData::GetLocalLanguage());

		Modio::Detail::ParseSafe(Json, TagInfo.bHidden, "hidden");
		Modio::Detail::ParseSafe(Json, TagInfo.bLocked, "locked");
	}

	inline void from_json(const nlohmann::json& Json, Modio::ModTagOptions& Options)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(Options));
		Detail::ParseSafe(Json, Options.InternalList, "data");
	}

} // namespace Modio
