/*
 *  Copyright (C) 2021-2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/core/ModioCoreTypes.h"
#endif

#include "platform/PlatformUtilImplementation.h"

namespace Modio
{
	namespace Detail
	{
		std::string ToString(Modio::PlatformStatus Status)
		{
			switch (Status)
			{
				case PlatformStatus::PendingOnly:
					return "pending_only";
				case PlatformStatus::LiveAndPending:
					return "live_and_pending";
				case PlatformStatus::ApprovedOnly:
					return "approved_only";
			}

			assert(false && "Invalid value to ToString(Modio::PlatformStatus)");
			return "Unknown";
		}

		std::string ToString(Modio::Language Locale)
		{
			switch (Locale)
			{
				case Language::English:
					return "en";
				case Language::Bulgarian:
					return "bg";
				case Language::French:
					return "fr";
				case Language::German:
					return "de";
				case Language::Italian:
					return "it";
				case Language::Polish:
					return "pl";
				case Language::Portuguese:
					return "pt";
				case Language::Hungarian:
					return "hu";
				case Language::Japanese:
					return "ja";
				case Language::Korean:
					return "ko";
				case Language::Russian:
					return "ru";
				case Language::Spanish:
					return "es";
				case Language::SpanishLatinAmerican:
					return "es-419";
				case Language::Thai:
					return "th";
				case Language::ChineseSimplified:
					return "zh-CN";
				case Language::ChineseTraditional:
					return "zh-TW";
				case Language::Turkish:
					return "tr";
				case Language::Ukrainian:
					return "uk";
				case Language::Arabic:
					return "ar";
				case Language::Czech:
					return "cs";
				case Language::PortugueseBrazilian:
					return "pt-BR";
			}
			assert(false && "Invalid value to ToString(Modio::Language)");
			return "Unknown";
		}

		std::string ToString(Modio::AuthenticationProvider Provider)
		{
			switch (Provider)
			{
				case AuthenticationProvider::XboxLive:
					return "xbox";
				case AuthenticationProvider::Steam:
					return "steam";
				case AuthenticationProvider::GoG:
					return "gog";
				case AuthenticationProvider::Itch:
					return "itchio";
				case AuthenticationProvider::Switch:
					return "switch";
				case AuthenticationProvider::Discord:
					return "discord";
				case AuthenticationProvider::PSN:
					return "psn";
				case AuthenticationProvider::Oculus:
					return "oculus";
				case AuthenticationProvider::Epic:
					return "epic";
				case AuthenticationProvider::OpenID:
					return "openid";
				case AuthenticationProvider::Apple:
					return "apple";
				case AuthenticationProvider::GoogleIDToken:
					return "googleidtoken";
				case AuthenticationProvider::GoogleServerSideToken:
					return "googleserversidetoken";
				default:
					return "none";
			}
		}
	} // namespace Detail
} // namespace Modio