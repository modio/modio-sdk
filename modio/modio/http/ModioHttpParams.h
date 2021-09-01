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
#include "ModioGeneratedVariables.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ModioStringHelpers.h"
#include "modio/detail/http/ModioRequestBodyKVPContainer.h"
#include <regex>
#include <string>
#undef DELETE

namespace Modio
{
	namespace Detail
	{
		enum class Verb
		{
			GET,
			POST,
			HEAD,
			PUT,
			DELETE
		};
		enum class APIVersion
		{
			V1
		};

		class HttpRequestParams
		{
		public:
			// that mutates the object instead to avoid memory allocations as it will be potentially expensive to copy
			// the object when setting a payload
			MODIO_IMPL HttpRequestParams SetGameID(GameID ID) const;

			MODIO_IMPL HttpRequestParams& SetGameID(GameID ID);

			MODIO_IMPL HttpRequestParams SetModID(std::uint32_t ID) const;

			MODIO_IMPL HttpRequestParams& SetModID(std::uint32_t ID);

			MODIO_IMPL HttpRequestParams SetFilterString(const std::string& InFilterString) const;

			MODIO_IMPL HttpRequestParams& SetFilterString(const std::string& InFilterString);

			MODIO_IMPL HttpRequestParams SetLocale(const Modio::Language Locale) const;

			MODIO_IMPL HttpRequestParams& SetLocale(const Modio::Language Locale);

			// Overload to silently drop nulled keys - doesn't have to be a template strictly, but is one so that it's
			// lower priority than the std::string version in the overload set
			template<typename UnderlyingType>
			HttpRequestParams AppendPayloadValue(std::string Key, Modio::Optional<UnderlyingType> Value) const
			{
				if (Value)
				{
					return AppendPayloadValue(Key, *Value);
				}
				else
				{
					return HttpRequestParams(*this);
				}
			}

			MODIO_IMPL HttpRequestParams AppendPayloadValue(std::string Key, std::string Value) const;

			template<typename T>
			HttpRequestParams SetPayload(T RawPayload) const
			{
				auto NewParamsInstance = HttpRequestParams(*this);
				NewParamsInstance.SetPayload(RawPayload);
				return NewParamsInstance;
			}

			/// @brief Sets the payload of a HTTP request from some raw data object. Calls
			/// ToRequestBody(Modio::Detail::RequestBodyKVPContainer& Container, const T& Payload) to perform the
			/// serialization
			/// @tparam T Type of the object that we're wanting to convert
			/// @param RawPayload Object instance we want to convert 
			/// @return The modified Request Params object with the payload set
			template<typename T>
			HttpRequestParams& SetPayload(T RawPayload)
			{
				// Get requests can't have payloads
				if (GetTypedVerb() == Verb::GET)
				{
					Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::Http,
												"Can't add any payload to GET request [{}], silently failing",
												ResourcePath);
					return *this;
				}

				Modio::Detail::RequestBodyKVPContainer KVPValues;
				ToRequestBody(KVPValues, RawPayload);

				std::string TempPayload;
				bool bFirstIteration = true;
				for (const auto& Itr : KVPValues)
				{
					if (!bFirstIteration)
					{
						TempPayload += "&";
					}
					else
					{
						bFirstIteration = false;
					}

					TempPayload += fmt::format("{}={}", Itr.first, Modio::Detail::String::URLEncode(Itr.second));
				}
				Payload = std::move(TempPayload);

				return *this;
			}

			MODIO_IMPL HttpRequestParams SetAuthTokenOverride(const std::string& AuthToken) const;

			MODIO_IMPL HttpRequestParams& SetAuthTokenOverride(const std::string& AuthToken);

			MODIO_IMPL HttpRequestParams SetRange(Modio::FileOffset Start,
												  Modio::Optional<Modio::FileOffset> End) const;

			MODIO_IMPL HttpRequestParams& SetRange(Modio::FileOffset Start, Modio::Optional<Modio::FileOffset> End);

			MODIO_IMPL std::string GetServerAddress() const;

			MODIO_IMPL std::string GetFormattedResourcePath() const;

			MODIO_IMPL std::string GetVerb() const;

			MODIO_IMPL Verb GetTypedVerb() const;

			MODIO_IMPL const Optional<std::string>& GetPayload() const;

			using Header = std::pair<std::string, std::string>;
			using HeaderList = std::vector<Header>;

			MODIO_IMPL HeaderList GetHeaders() const;

			HttpRequestParams(Modio::Detail::Verb CurrentOperationType, const char* ResourcePath)
				: ResourcePath(ResourcePath),
				  GameID(0),
				  ModID(0),
				  CurrentOperationType(CurrentOperationType),
				  CurrentAPIVersion(Modio::Detail::APIVersion::V1)
			{}

			HttpRequestParams()
				: ResourcePath("NOT_SET"),
				  GameID(0),
				  ModID(0),
				  CurrentOperationType(Modio::Detail::Verb::GET),
				  CurrentAPIVersion(Modio::Detail::APIVersion::V1)
			{}

			MODIO_IMPL Modio::Detail::Buffer GetRequestBuffer(bool bPerformURLEncoding = false) const;

			MODIO_IMPL static Modio::Optional<HttpRequestParams> FileDownload(std::string URL);
			MODIO_IMPL Modio::Optional<Modio::Detail::HttpRequestParams> RedirectURL(std::string URL) const;

		private:
			MODIO_IMPL HttpRequestParams(std::string Server, std::string ResourcePath);
			MODIO_IMPL std::string GetAPIVersionString() const;
			MODIO_IMPL std::string GetAPIKeyString() const;

			MODIO_IMPL const Modio::Optional<std::string> GetAuthToken() const;

			/// @brief Resolves all placeholder parameters in a resource path to their actual values
			/// @return The fully-resolved resource path
			MODIO_IMPL std::string GetResolvedResourcePath() const;

			bool bFileDownload = false;

			std::string FileDownloadServer;

			std::string ResourcePath;
			std::uint64_t GameID;
			std::uint64_t ModID;

			// @todo: Investigate FilterString/Payload how we could refactor those
			Modio::Optional<std::string> FilterString;
			// This should most likely be a ID into a separate payload store or
			// let it be put as a different parameter
			Modio::Optional<std::string> Payload;
			Modio::Optional<std::string> AuthTokenOverride;

			// Temporary workaround for specifying the locale on Terms request
			Modio::Optional<Modio::Language> OverrideLocale;

			std::string APIKey;

			// current API key
			// URL
			Verb CurrentOperationType;
			// other headers
			// payload data
			// a lot of this data can be constexpr
			// api version
			APIVersion CurrentAPIVersion;

			Modio::Optional<Modio::FileOffset> StartOffset;
			Modio::Optional<Modio::FileOffset> EndOffset;
		};

// TODO: @Modio-core implement comparison operator for HttpRequestParams and maybe IsValid()?
#ifdef MODIO_SEPARATE_COMPILATION
		extern HttpRequestParams InvalidParams;
#else
		MODIO_IMPL HttpRequestParams InvalidParams;
#endif
	} // namespace Detail
}; // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioHttpParams.ipp"
#endif

#include "modio/core/ModioDefaultRequestParameters.h"