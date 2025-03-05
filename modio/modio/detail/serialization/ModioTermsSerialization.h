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

#include "modio/core/entities/ModioTerms.h"

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::Terms& OutTerms)
	{
		nlohmann::json ButtonsJson;
		if (Detail::GetSubobjectSafe(Json, "buttons", ButtonsJson))
		{
			Detail::ParseSubobjectSafe(ButtonsJson, OutTerms.Buttons.AgreeText, "agree", "text");
			Detail::ParseSubobjectSafe(ButtonsJson, OutTerms.Buttons.DisagreeText, "disagree", "text");
		}

		Detail::ParseSubobjectSafe(Json, OutTerms.Links.Website, "links", "website");
		Detail::ParseSubobjectSafe(Json, OutTerms.Links.Terms, "links", "terms");
		Detail::ParseSubobjectSafe(Json, OutTerms.Links.Privacy, "links", "privacy");
		Detail::ParseSubobjectSafe(Json, OutTerms.Links.Refund, "links", "refund");
		Detail::ParseSubobjectSafe(Json, OutTerms.Links.Manage, "links", "manage");

		Detail::ParseSafe(Json, OutTerms.TermsText, "plaintext");
	}

	inline void from_json(const nlohmann::json& Json, Modio::Terms::Link& OutLink)
	{
		Detail::ParseSafe(Json, OutLink.Text, "text");
		Detail::ParseSafe(Json, OutLink.URL, "url");
		Detail::ParseSafe(Json, OutLink.bRequired, "required");
	}
} // namespace Modio
