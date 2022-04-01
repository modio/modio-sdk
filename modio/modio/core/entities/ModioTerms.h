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
#include "modio/detail/ModioJsonHelpers.h"
namespace Modio
{
	/// @brief This struct contains strings that should be displayed to a user
	/// when displaying the terms of use and offering to create a mod.io account
	struct Terms
	{
	public:
		struct Link
		{
			/// @brief The user-facing text for the link
			std::string Text;
			/// @brief The actual URL for the link
			std::string URL;
			/// @brief Is displaying this link mandatory?
			bool bRequired;
		};

		/// @docpublic
		struct TermsButtons
		{
			/// @brief Text to display on the affirmative/OK button
			std::string AgreeText;
			/// @brief Text to display on the negative/cancel button
			std::string DisagreeText;
		} Buttons;

		/// @docpublic
		struct TermsLinks
		{
			/// @brief Link to the mod.io website
			Link Website;
			/// @brief Link to the mod.io terms of use
			Link Terms;
			/// @brief Link to the mod.io Privacy Policy
			Link Privacy;
			/// @brief Link to the mod.io Manage User Account page
			Link Manage;
		} Links;

		/// @brief The plaintext version of the mod.io terms of use
		std::string TermsText;

		friend bool operator==(const Modio::Terms& A, const Modio::Terms& B)
		{
			if (A.Buttons.AgreeText == B.Buttons.AgreeText && A.Buttons.DisagreeText == B.Buttons.DisagreeText &&
				A.Links.Website.Text == B.Links.Website.Text && A.Links.Website.URL == B.Links.Website.URL &&
				A.Links.Website.bRequired == B.Links.Website.bRequired && A.Links.Terms.Text == B.Links.Terms.Text &&
				A.Links.Terms.URL == B.Links.Terms.URL && A.Links.Terms.bRequired == B.Links.Terms.bRequired &&
				A.Links.Privacy.Text == B.Links.Privacy.Text && A.Links.Privacy.URL == B.Links.Privacy.URL &&
				A.Links.Privacy.bRequired == B.Links.Privacy.bRequired && A.Links.Manage.Text == B.Links.Manage.Text &&
				A.Links.Manage.URL == B.Links.Manage.URL && A.Links.Manage.bRequired == B.Links.Manage.bRequired &&
				A.TermsText == B.TermsText)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	inline void from_json(const nlohmann::json& Json, Modio::Terms& OutTerms)
	{
		nlohmann::json ButtonsJson;
		if(Detail::GetSubobjectSafe(Json, "buttons", ButtonsJson))
		{
			Detail::ParseSubobjectSafe(ButtonsJson, OutTerms.Buttons.AgreeText, "agree", "text");
			Detail::ParseSubobjectSafe(ButtonsJson, OutTerms.Buttons.DisagreeText, "disagree", "text");
		}
		
		Detail::ParseSubobjectSafe(Json, OutTerms.Links.Website, "links", "website");
		Detail::ParseSubobjectSafe(Json, OutTerms.Links.Terms, "links", "terms");
		Detail::ParseSubobjectSafe(Json, OutTerms.Links.Privacy, "links", "privacy");
		Detail::ParseSubobjectSafe(Json, OutTerms.Links.Manage, "links", "manage");

		Detail::ParseSafe(Json, OutTerms.TermsText, "plaintext");
	}
	
	inline void from_json(const nlohmann::json& Json, Modio::Terms::Link& OutLink)
	{
		Detail::ParseSafe(Json, OutLink.Text, "text");
		Detail::ParseSafe(Json, OutLink.URL, "url");
		Detail::ParseSafe(Json, OutLink.bRequired, "required");
	}

}