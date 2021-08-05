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
			}
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
			}
		}
	}
}
