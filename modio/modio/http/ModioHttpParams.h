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
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ModioStringHash.h"
#include "modio/detail/ModioStringHelpers.h"
#include "modio/detail/http/ModioRequestBodyKVPContainer.h"
#include <regex>
#include <string>
#undef DELETE

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Enum with HTTP verbs
		enum class Verb
		{
			GET,
			POST,
			HEAD,
			PUT,
			DELETE
		};

		/// @docinternal
		/// @brief Enum with the API version in use
		enum class APIVersion
		{
			V1
		};

		/// @brief Enumeration of the possible Cache actions, to either
		/// keep information of a request or not
		enum class CachedResponse : std::uint8_t
		{
			Allow,
			Disallow
		};	

		/// @docinternal
		/// @brief Container for payloads retrieved/uploaded by HTTP operations
		struct PayloadContent
		{
			enum class PayloadType : std::uint8_t
			{
				Buffer,
				File,
				FilePortion
			};

			Modio::Optional<Modio::Detail::Buffer> RawBuffer;
			Modio::Optional<Modio::filesystem::path> PathToFile;
			PayloadType PType = PayloadType::Buffer;
			// This is the amount of bytes the file occupies in the system
			Modio::FileSize FileSize = Modio::FileSize(0);
			// If the content should consider an offset from the file
			Modio::FileOffset Offset = Modio::FileOffset(0);
			// If file should have a maximum number of bytes for the content
			// By default, it would be the same as FileSize, unless
			Modio::FileSize ContentSize = Modio::FileSize(0);

			// Prevent default construction, but allow copying to duplicate the buffer if it's present, and allow
			// moving, too This maintains the copyability of HttpRequestParams
			MODIO_IMPL PayloadContent(Modio::Detail::Buffer InRawBuffer);
			MODIO_IMPL PayloadContent(Modio::filesystem::path PathToFile, Modio::FileSize FSize);
			MODIO_IMPL PayloadContent(Modio::filesystem::path PathToFile, Modio::FileSize FSize,
									  Modio::FileOffset FOffset, Modio::FileSize ContentSize);
			MODIO_IMPL PayloadContent(const PayloadContent& Other);
			MODIO_IMPL PayloadContent& operator=(const PayloadContent& Other);
			MODIO_IMPL PayloadContent(PayloadContent&& Other) = default;
			MODIO_IMPL PayloadContent& operator=(PayloadContent&& Other);
            
            // Search in "Members" for any attribute that has a PayloadContent with
            // ContentSize != 0 and add that to the result "FileSize"
			MODIO_IMPL static Modio::Optional<Modio::FileSize> PayloadContentSize(
				std::map<std::string, PayloadContent> Members);
		};
		
		/// @docinternal
		/// @brief Set of operations to construct HTTP requests
		class HttpRequestParams
		{
		public:
			// that mutates the object instead to avoid memory allocations as it will be potentially expensive to copy
			// the object when setting a payload
			MODIO_IMPL HttpRequestParams SetGameID(GameID ID) const;

			MODIO_IMPL HttpRequestParams& SetGameID(GameID ID);

			MODIO_IMPL HttpRequestParams SetModID(Modio::ModID ID) const;

			MODIO_IMPL HttpRequestParams& SetModID(Modio::ModID ID);

			MODIO_IMPL HttpRequestParams SetUserID(Modio::UserID ID) const;

			MODIO_IMPL HttpRequestParams& SetUserID(Modio::UserID ID);

			MODIO_IMPL HttpRequestParams AddQueryParamRaw(const std::string& Key, const std::string& Value) const;
			MODIO_IMPL HttpRequestParams& AddQueryParamRaw(const std::string& Key, const std::string& Value);

			MODIO_IMPL HttpRequestParams AddHeaderRaw(const std::string& Key, const std::string& Value) const;
			MODIO_IMPL HttpRequestParams& AddHeaderRaw(const std::string& Key, const std::string& Value);

			MODIO_IMPL HttpRequestParams AddLimitQueryParam() const;

			MODIO_IMPL HttpRequestParams AddOffsetQueryParam(int32_t offset) const;

			MODIO_IMPL HttpRequestParams AddCurrentGameIdQueryParam() const;

			MODIO_IMPL HttpRequestParams AddPlatformStatusFilter() const;

			MODIO_IMPL HttpRequestParams AddStatusFilter() const;

			MODIO_IMPL HttpRequestParams AppendQueryParameterMap(const std::map<std::string, std::string> InQueryParameterMap) const;


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

			// Overload "EncodeAndAppendPayloadValue" to silently drop nulled keys.
			MODIO_IMPL HttpRequestParams EncodeAndAppendPayloadValue(std::string Key,
																	 Modio::Optional<std::string> Value) const;

			// This function will URL encode the "value" string before adding it to HttpRequestParams
			MODIO_IMPL HttpRequestParams EncodeAndAppendPayloadValue(std::string Key, std::string Value) const;

			MODIO_IMPL HttpRequestParams AppendPayloadValue(std::string Key, std::string Value) const;

			MODIO_IMPL HttpRequestParams AppendEmptyPayload(std::string Key) const;

			MODIO_IMPL HttpRequestParams AppendPayloadValue(std::string Key,
															Modio::Detail::Buffer RawPayloadBuffer) const;

			MODIO_IMPL HttpRequestParams& AppendPayloadValue(std::string Key, Modio::Detail::Buffer RawPayloadBuffer);

			MODIO_IMPL HttpRequestParams AppendJsonPayloadValue(std::string Payload) const;

			// TODO: @modio-core consider if this should return Modio::Optional instead. Breaks method chaining but we
			// probably want to fail early
			MODIO_IMPL HttpRequestParams AppendPayloadFile(std::string Key, Modio::filesystem::path PathToFileToUpload,
														   Modio::Optional<Modio::FileOffset> FileOffset = {},
														   Modio::Optional<Modio::FileSize> ContentSize = {}) const;

			// @brief Template function to append a payload to a new instance of HTTPRequestParams
			template<typename T>
			HttpRequestParams SetPayload(T RawPayload) const
			{
				HttpRequestParams NewParamsInstance(*this);
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
				MODIO_PROFILE_SCOPE(HttpParamsSetPayload);
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

			// Suppress sending the X-Modio-Platform header for this request
			MODIO_IMPL HttpRequestParams SuppressPlatformHeader() const;

			// Suppress sending the X-Modio-Platform header for this request
			MODIO_IMPL HttpRequestParams& SuppressPlatformHeader();

			// It would enable the HTTP header "Range: bytes=Start-End"
			MODIO_IMPL HttpRequestParams SetRange(Modio::FileOffset Start,
												  Modio::Optional<Modio::FileOffset> End) const;

			// It would enable the HTTP header "Range: bytes=Start-End"
			MODIO_IMPL HttpRequestParams& SetRange(Modio::FileOffset Start, Modio::Optional<Modio::FileOffset> End);
			
			// It would enable the HTTP header "ContentRange: bytes Start-End/TotalBytes"
			MODIO_IMPL HttpRequestParams& SetContentRange(Modio::FileOffset Start, Modio::FileOffset End,
														  Modio::FileOffset TotalBytes);

			MODIO_IMPL std::string GetServerAddress() const;

			MODIO_IMPL std::string GetFormattedResourcePath() const;

			MODIO_IMPL std::string GetVerb() const;

			MODIO_IMPL Verb GetTypedVerb() const;

			MODIO_IMPL bool ContainsFormData() const;

			/// @brief Gets the URL-encoded payload for the request
			/// @return Optional string containing the URLEncoded payload IF the entire payload supports URLencoding (ie
			/// there are no file payload parameters)
			MODIO_IMPL const Optional<std::string> GetUrlEncodedPayload() const;

			MODIO_IMPL Modio::FileSize GetPayloadSize() const;

			MODIO_IMPL Modio::Optional<std::pair<std::string, Modio::Detail::PayloadContent>> TakeNextPayloadElement();

			using Header = std::pair<std::string, std::string>;
			using HeaderList = std::vector<Header>;

			MODIO_IMPL HeaderList GetHeaders() const;

			HttpRequestParams(Modio::Detail::Verb CurrentOperationType, const char* ResourcePath,
							  const char* ContentType)
				: ResourcePath(ResourcePath),
				  ContentType(ContentType),
				  GameID(0),
				  ModID(0),
				  UserID(0),
				  CurrentOperationType(CurrentOperationType),
				  CurrentAPIVersion(Modio::Detail::APIVersion::V1) {}

			HttpRequestParams(Modio::Detail::Verb CurrentOperationType, const char* ResourcePath)
				: ResourcePath(ResourcePath),
				  ContentType(),
				  GameID(0),
				  ModID(0),
				  UserID(0),
				  CurrentOperationType(CurrentOperationType),
				  CurrentAPIVersion(Modio::Detail::APIVersion::V1)
			{}

			HttpRequestParams()
				: ResourcePath("NOT_SET"),
				  ContentType(),
				  GameID(0),
				  ModID(0),
				  UserID(0),
				  CurrentOperationType(Modio::Detail::Verb::GET),
				  CurrentAPIVersion(Modio::Detail::APIVersion::V1)
			{}

			MODIO_IMPL Modio::Detail::Buffer GetRequestBuffer() const;
			MODIO_IMPL static Modio::Optional<HttpRequestParams> FileDownload(
				std::string URL, Modio::Optional<Modio::Detail::HttpRequestParams> FromRedirect = {});

			static constexpr std::uint64_t GetBoundaryHash()
			{
				return "modio_sdk_multipart_boundary"_hash;
			}

			MODIO_IMPL void SetUserAgentOverride(std::string UserAgentHeader);

		private:
			MODIO_IMPL HttpRequestParams(std::string Server, std::string ResourcePath);
			MODIO_IMPL std::string GetAPIVersionString() const;
			MODIO_IMPL std::string GetAPIKeyString() const;

			MODIO_IMPL const Modio::Optional<std::string> GetAuthToken() const;

			/// @brief Resolves all placeholder in a resource path to their actual values, and build
			///	the query string for any QueryParameters that have been passed in
			/// @return The fully-resolved resource path
			MODIO_IMPL std::string GetResolvedResourcePath() const;

			bool bFileDownload = false;

			// Whether this request should suppress the X-Modio-Platform header
			bool bSuppressPlatformHeader = false;

			std::string FileDownloadServer = "";

			std::string ResourcePath = "";

			Modio::Optional<std::string> ContentType = {};

			std::uint64_t GameID;
			std::uint64_t ModID;
			std::uint64_t UserID;

			// This should most likely be a ID into a separate payload store or
			// let it be put as a different parameter
			Modio::Optional<std::string> Payload;

			std::map<std::string, PayloadContent> PayloadMembers;

			Modio::Optional<std::string> AuthTokenOverride;

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

			// Three variables needed to create the header Content-Range
			// {Start, End, Total}
			Modio::Optional<std::tuple<Modio::FileOffset, Modio::FileOffset, Modio::FileOffset>> ContentRangeOffsets;
			
			Modio::Optional<std::string> UserAgentOverride;

			std::map<std::string, std::string> QueryParameters;

			std::map<std::string, std::string> AdditionalHeaders;
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
