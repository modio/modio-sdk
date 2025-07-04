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
				constexpr const char* AuthCode = "auth_code";
				constexpr const char* SecurityCode = "security_code";
				constexpr const char* ItchToken = "itchio_token";
				constexpr const char* XboxToken = "xbox_token";
				constexpr const char* SwitchToken = "id_token";
				constexpr const char* OpenIDToken = "id_token";
				constexpr const char* GoogleIDToken = "id_token";
				constexpr const char* GoogleServerSideToken = "auth_code";
				constexpr const char* DiscordToken = "discord_token";
				constexpr const char* Appdata = "appdata";
				constexpr const char* EmailAddress = "email";
				constexpr const char* TermsAgreed = "terms_agreed";
				constexpr const char* Receipt = "receipt";
				constexpr const char* ReportResourceType = "resource";
				constexpr const char* ReportResourceID = "id";
				constexpr const char* ReportType = "type";
				constexpr const char* ReportSubmitterName = "name";
				constexpr const char* ReportSubmitterContact = "contact";
				constexpr const char* ReportSummary = "summary";
				constexpr const char* Rating = "rating";
				constexpr const char* GameID = "game_id";
				constexpr const char* Device = "device";
				constexpr const char* UserID = "user_id";
				constexpr const char* Nonce = "nonce";
				constexpr const char* AccessToken = "access_token";
				constexpr const char* UploadID = "upload_id";
			} // namespace APIStrings
			namespace QueryParamStrings
			{
				constexpr const char* Recursive = "recursive";
				constexpr const char* IncludeDependecies = "include_dependencies";
				constexpr const char* ShowHiddenTags = "show_hidden_tags";
			} // namespace QueryParamStrings
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
				constexpr const char* WalletBalance = "balance";
				constexpr const char* GrossAmount = "gross_amount";
				constexpr const char* TokenEntity = "entity";
				constexpr const char* TokenValue = "token";
			} // namespace JSONKeys
			namespace Configuration
			{
				constexpr uint8_t DefaultNumberOfRetries = 3;
				constexpr auto PollInterval = std::chrono::microseconds(100);
				// The maximum size a file can be before it is uploaded using a multipart operation
				// For reference, these are 200MiB = 209MB
				constexpr uintmax_t MultipartMaxFileSize = 209715200;
				// The maximum size a file section to upload using a multipart operation
				// For reference, these are 50MiB = 52MB
				constexpr uintmax_t MultipartMaxFilePartSize = 52428800;
				// A heartbeat POST request is required to be submitted at-most every 5 minutes (300s).
				// We send a heartbeat by default at half that requirement to ensure we do not time out.
				constexpr uint32_t MetricsHeartbeatIntervalSeconds = 150;
				// The maximum path length (260 characters) defined by the Win32 API as MAX_PATH.
				// This value is enforced to ensure compatibility across platforms, particularly
				// preventing issues on Windows where path lengths are restricted.
				// By applying this universally, we avoid mod creation on platforms without the limit
				// that would otherwise encounter issues when run on Windows systems.
				constexpr std::size_t UniversalMaxPath = 260;
			} // namespace Configuration
			namespace PlatformNames
			{
				constexpr const char* Windows = "windows";
				constexpr const char* Mac = "mac";
				constexpr const char* Linux = "linux";
				constexpr const char* Android = "android";
				constexpr const char* iOS = "ios";
				constexpr const char* XboxOne = "xboxone";
				constexpr const char* XboxSeriesX = "xboxseriesx";
				constexpr const char* PS4 = "ps4";
				constexpr const char* PS5 = "ps5";
				constexpr const char* Switch = "switch";
				constexpr const char* Oculus = "oculus";
				constexpr const char* Source = "source";
			} // namespace PlatformNames
		} // namespace Constants
	} // namespace Detail
} // namespace Modio
