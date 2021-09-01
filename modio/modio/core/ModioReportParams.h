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
#include "modio/core/ModioCoreTypes.h"

namespace Modio
{
	namespace Detail
	{
		class HttpRequestParams;
	} // namespace Detail

	class ReportParams;

	/// @docpublic
	/// @brief The type of report being made
	enum class ReportType : uint8_t
	{
		Generic = 0,
		DMCA = 1,
		NotWorking = 2,
		RudeContent = 3,
		IllegalContent = 4,
		StolenContent = 5,
		FalseInformation = 6,
		Other = 7
	};

	/// @docpublic
	/// @brief Class containing data to be submitted in a content report
	class ReportParams
	{
	public:
		/// @brief Creates a content report for a game.
		/// @param Game The ID of the game being reported
		/// @param Type The nature of the content report
		/// @param ReportDescription A description of why the content is being reported
		/// @param ReporterName Name of the submitting user. Recommended for DMCA reports, but may be empty
		/// @param ReporterContact Contact details of the submitting user. Recommended for DMCA reports, but may be
		/// empty
		MODIO_IMPL ReportParams(Modio::GameID Game, Modio::ReportType Type, std::string ReportDescription,
								Modio::Optional<std::string> ReporterName,
								Modio::Optional<std::string> ReporterContact);

		/// @docpublic
		/// @brief Creates a content report for a game.
		/// @param User The ID of the User being reported
		/// @param Type The nature of the content report
		/// @param ReportDescription A description of why the content is being reported
		/// @param ReporterName Name of the submitting user. Recommended for DMCA reports, but may be empty
		/// @param ReporterContact Contact details of the submitting user. Recommended for DMCA reports, but may be
		/// empty
		MODIO_IMPL ReportParams(Modio::UserID User, Modio::ReportType Type, std::string ReportDescription,
								Modio::Optional<std::string> ReporterName,
								Modio::Optional<std::string> ReporterContact);

		/// @docpublic
		/// @brief Creates a content report for a game.
		/// @param Mod The ID of the content being reported
		/// @param Type The nature of the content report
		/// @param ReportDescription A description of why the content is being reported
		/// @param ReporterName Name of the submitting user. Recommended for DMCA reports, but may be empty
		/// @param ReporterContact Contact details of the submitting user. Recommended for DMCA reports, but may be
		/// empty
		MODIO_IMPL ReportParams(Modio::ModID Mod, Modio::ReportType Type, std::string ReportDescription,
								Modio::Optional<std::string> ReporterName,
								Modio::Optional<std::string> ReporterContact);

	private:
		Modio::Optional<std::string> ReporterName;
		Modio::Optional<std::string> ReporterContact;
		std::string ReportDescription;
		enum class ResourceType : uint8_t
		{
			Game,
			Mod,
			User
		};
		ResourceType ReportedResourceType;
		/// @brief Type-erased storage for the underlying resource ID. NB if REST API changes ID types this will
		/// need to be altered to match
		std::int64_t ResourceID;

		ReportType Type;

		MODIO_IMPL ReportParams(std::int64_t ResourceID, ResourceType ReportedResourceType, Modio::ReportType Type,
								std::string ReportDescription, Modio::Optional<std::string> ReporterName,
								Modio::Optional<std::string> ReporterContact);
		/// @docinternal
		/// @brief returns the http request parameters for a report submission, with the payload data set
		/// @param Params the report to submit
		/// @return the request parameters for use with PerformRequestAndGetResponseAsync
		MODIO_IMPL friend Modio::Detail::HttpRequestParams ToRequest(const Modio::ReportParams& Params);
	};

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioReportParams.ipp"
#endif