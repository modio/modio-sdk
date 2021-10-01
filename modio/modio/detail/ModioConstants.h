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

namespace Modio
{
	namespace Detail
	{
		namespace Constants
		{
			namespace APIStrings
			{
				constexpr const char* SecurityCode = "security_code";
				constexpr const char* ItchToken = "itchio_token";
				constexpr const char* XboxToken = "xbox_token";
				constexpr const char* SwitchToken = "id_token";
				constexpr const char* DiscordToken = "discord_token";
				constexpr const char* Appdata = "appdata";
				constexpr const char* EmailAddress = "email";
				constexpr const char* TermsAgreed = "terms_agreed";
				constexpr const char* ReportResourceType = "resource";
				constexpr const char* ReportResourceID = "id";
				constexpr const char* ReportType = "type";
				constexpr const char* ReportSubmitterName = "name";
				constexpr const char* ReportSubmitterContact = "contact";
				constexpr const char* ReportSummary = "summary";
				constexpr const char* Rating = "rating";
			} // namespace APIStrings
			namespace JSONKeys
			{
				constexpr const char* UserSubscriptionList = "subscriptions";
				constexpr const char* ModCollection = "Mods";
				constexpr const char* DeferredUnsubscribes = "DeferredUnsubscribes";
				constexpr const char* ModEntryID = "ID";
				constexpr const char* OAuth = "OAuth";
				constexpr const char* UserProfile = "Profile";
				constexpr const char* Avatar = "Avatar";
				constexpr const char* ModEntryProfile = "Profile";
				constexpr const char* ModEntrySubCount = "SubscriptionCount";
				constexpr const char* ModEntryState = "State";
				constexpr const char* ModSizeOnDisk = "SizeOnDisk";
				constexpr const char* ModPathOnDisk = "PathOnDisk";
				constexpr const char* RootLocalStoragePath = "RootLocalStoragePath";
				constexpr const char* ModNeverRetryCategory = "NeverRetryCategory";
				constexpr const char* ModNeverRetryCode = "NeverRetryCode";
			}
			namespace Configuration
			{
				constexpr uint8_t DefaultNumberOfRetries = 3;
			}
		}
	}
}
