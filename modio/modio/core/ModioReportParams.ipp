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
	#include "modio/core/ModioReportParams.h"
#endif

#include "modio/detail/ModioConstants.h"
#include "modio/http/ModioHttpParams.h"

namespace Modio
{
	ReportParams::ReportParams(std::int64_t ResourceID, ResourceType ReportedResourceType, Modio::ReportType Type,
							   std::string ReportDescription, Modio::Optional<std::string> ReporterName,
							   Modio::Optional<std::string> ReporterContact)
		: ReporterName(ReporterName),
		  ReporterContact(ReporterContact),
		  ReportDescription(ReportDescription),
		  ReportedResourceType(ReportedResourceType),
		  ResourceID(ResourceID),
		  Type(Type)
	{}

	ReportParams::ReportParams(Modio::GameID Game, Modio::ReportType Type, std::string ReportDescription,
							   Modio::Optional<std::string> ReporterName, Modio::Optional<std::string> ReporterContact)
		: ReportParams(Game, ResourceType::Game, Type, ReportDescription, ReporterName, ReporterContact)
	{}

	ReportParams::ReportParams(Modio::ModID Mod, Modio::ReportType Type, std::string ReportDescription,
							   Modio::Optional<std::string> ReporterName, Modio::Optional<std::string> ReporterContact)
		: ReportParams(Mod, ResourceType::Mod, Type, ReportDescription, ReporterName, ReporterContact)
	{}

	ReportParams::ReportParams(Modio::UserID User, Modio::ReportType Type, std::string ReportDescription,
							   Modio::Optional<std::string> ReporterName, Modio::Optional<std::string> ReporterContact)
		: ReportParams(User, ResourceType::User, Type, ReportDescription, ReporterName, ReporterContact)
	{}

	ReportParams::ReportParams(Modio::ModCollectionID Mod, Modio::ReportType Type, std::string ReportDescription,
							   Modio::Optional<std::string> ReporterName, Modio::Optional<std::string> ReporterContact)
		: ReportParams(Mod, ResourceType::Collection, Type, ReportDescription, ReporterName, ReporterContact)
	{}

	Modio::Detail::HttpRequestParams ToRequest(const Modio::ReportParams& Params)
	{
		std::string ResourceString;
		switch (Params.ReportedResourceType)
		{
			case ReportParams::ResourceType::Game:
				ResourceString = "games";
				break;
			case ReportParams::ResourceType::Mod:
				ResourceString = "mods";
				break;
			case ReportParams::ResourceType::User:
				ResourceString = "users";
				break;
			case ReportParams::ResourceType::Collection:
				ResourceString = "collections";
				break;
		}

		Modio::Detail::HttpRequestParams Request = Modio::Detail::SubmitReportRequest;
		if (Params.ReporterName)
		{
			Request.AppendPayloadValue(Modio::Detail::Constants::APIStrings::ReportSubmitterName,
									   Params.ReporterName.value());
		}
		if (Params.ReporterContact)
		{
			Request.AppendPayloadValue(Modio::Detail::Constants::APIStrings::ReportSubmitterContact,
									   Params.ReporterContact.value());
		}

		return Request.AppendPayloadValue(Modio::Detail::Constants::APIStrings::ReportResourceType, ResourceString)
			.AppendPayloadValue(Modio::Detail::Constants::APIStrings::ReportResourceID,
								fmt::format("{}", Params.ResourceID))
			.AppendPayloadValue(Modio::Detail::Constants::APIStrings::ReportType,
								fmt::format("{}", static_cast<uint8_t>(Params.Type)))
			.AppendPayloadValue(Modio::Detail::Constants::APIStrings::ReportSummary, Params.ReportDescription);
	}

	bool ReportParams::IsResourceIdValid() const
	{
		switch (ReportedResourceType)
		{
			case ResourceType::Game:
				return Modio::GameID::InvalidGameID() != Modio::GameID(ResourceID);
			case ResourceType::Mod:
				return Modio::ModID::InvalidModID() != Modio::ModID(ResourceID);
			case ResourceType::User:
				return Modio::UserID::InvalidUserID() != Modio::UserID(ResourceID);
			case ResourceType::Collection:
				return Modio::ModCollectionID::InvalidCollectionID() != Modio::ModCollectionID(ResourceID);
		}
		return false;
	}
} // namespace Modio
