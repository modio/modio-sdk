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
	#include "modio/http/ModioHttpParams.h"
#endif
#include "ModioGeneratedVariables.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ModioProfiling.h"

namespace Modio
{
	namespace Detail
	{
		/// @brief Helper to construct payload content from a raw buffer
		/// @param RawBuffer The buffer to submit
		/// @return PayloadContent wrapping the buffer
		MODIO_IMPL PayloadContent MakePayloadContent(Modio::Detail::Buffer RawBuffer)
		{
			return PayloadContent(std::move(RawBuffer));
		}

		/// @brief Helper to construct payload content referencing a file
		/// @param PathToFile The path to the file that will be submitted
		/// @return Optional payload content. Will be empty if the file doesn't exist or we could otherwise not retrieve
		/// the file size
		MODIO_IMPL Modio::Optional<PayloadContent> MakePayloadContent(
			Modio::filesystem::path PathToFile, Modio::Optional<Modio::FileOffset> FileOffset = {},
			Modio::Optional<Modio::FileSize> ContentSize = {})
		{
			Modio::ErrorCode ec;
			std::uintmax_t TmpSize = Modio::filesystem::file_size(PathToFile, ec);
			if (!ec)
			{
				if (FileOffset.has_value() == true && ContentSize.has_value() == true)
				{
					return PayloadContent(PathToFile, Modio::FileSize(TmpSize), FileOffset.value(),
										  ContentSize.value());
				}
				else
				{
					return PayloadContent(PathToFile, Modio::FileSize(TmpSize));
				}
			}
			else
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
											"Could not get size for file payload at {}, got error  = {}",
											PathToFile.generic_string(), ec.message());
				return {};
			}
		}

		Modio::Detail::HttpRequestParams& HttpRequestParams::SetGameID(Modio::GameID ID)
		{
			this->GameID = ID;
			return *this;
		}

		Modio::Detail::HttpRequestParams HttpRequestParams::SetGameID(Modio::GameID ID) const
		{
			auto NewParamsInstance = HttpRequestParams(*this);
			NewParamsInstance.GameID = ID;
			return NewParamsInstance;
		}

		Modio::Detail::HttpRequestParams& HttpRequestParams::SetModID(Modio::ModID ID)
		{
			ModID = ID;
			return *this;
		}

		Modio::Detail::HttpRequestParams HttpRequestParams::SetModID(Modio::ModID ID) const
		{
			auto NewParamsInstance = HttpRequestParams(*this);
			NewParamsInstance.ModID = ID;
			return NewParamsInstance;
		}

		Modio::Detail::HttpRequestParams HttpRequestParams::SetUserID(Modio::UserID ID) const
		{
			auto NewParamsInstance = HttpRequestParams(*this);
			NewParamsInstance.UserID= ID;
			return NewParamsInstance;
		}

		Modio::Detail::HttpRequestParams& HttpRequestParams::SetUserID(Modio::UserID ID)
		{
			this->UserID = ID;
			return *this;
		}


		Modio::Detail::HttpRequestParams HttpRequestParams::SetLocale(Modio::Language Locale) const
		{
			auto NewParamsInstance = HttpRequestParams(*this);
			NewParamsInstance.OverrideLocale = Locale;
			return NewParamsInstance;
		}

		Modio::Detail::HttpRequestParams& HttpRequestParams::SetLocale(Modio::Language Locale)
		{
			OverrideLocale = Locale;
			return *this;
		}

		Modio::Detail::HttpRequestParams& HttpRequestParams::SetFilterString(const std::string& InFilterString)
		{
			// If there was a provided filter string, append on & to ensure that we can append on the API key
			if (InFilterString.size())
			{
				// @todo: Move away this & as it makes it hard to follow the logic of how the URL is built
				FilterString = InFilterString + "&";
			}

			return *this;
		}

		HttpRequestParams HttpRequestParams::SetFilterString(const std::string& InFilterString) const
		{
			auto NewParamsInstance = HttpRequestParams(*this);
			NewParamsInstance.SetFilterString(InFilterString);
			return NewParamsInstance;
		}

		HttpRequestParams HttpRequestParams::EncodeAndAppendPayloadValue(std::string Key, Modio::Optional<std::string> Value) const
		{
			if (Value)
			{
				return EncodeAndAppendPayloadValue(Key, *Value);
			}
			else
			{
				return HttpRequestParams(*this);
			}
		}

		HttpRequestParams HttpRequestParams::EncodeAndAppendPayloadValue(std::string Key, std::string Value) const
		{
			// This line makes sure that any string value appended to the HttpRequest is URL encoded
            std::string StringValue = Modio::Detail::String::URLEncode(Value);
            return AppendPayloadValue(Key, StringValue);
		}

		HttpRequestParams HttpRequestParams::AppendPayloadValue(std::string Key, std::string Value) const
		{
            Modio::Detail::Buffer ValueBuffer(Value.length());
            std::copy(Value.begin(), Value.end(), ValueBuffer.begin());
			return AppendPayloadValue(Key, std::move(ValueBuffer));
		}

		HttpRequestParams HttpRequestParams::AppendPayloadValue(std::string Key,
																Modio::Detail::Buffer RawPayloadBuffer) const
		{
            HttpRequestParams NewParamsInstance = HttpRequestParams(*this);
            
            // No need to try to append an empty buffer
            if (RawPayloadBuffer.GetSize() == 0)
            {
                return HttpRequestParams(*this);
            }
            
			auto KeyIterator = NewParamsInstance.PayloadMembers.find(Key);
			if (KeyIterator != NewParamsInstance.PayloadMembers.end())
			{
				NewParamsInstance.PayloadMembers.erase(KeyIterator);
			}
			NewParamsInstance.PayloadMembers.emplace(Key, MakePayloadContent(std::move(RawPayloadBuffer)));
			return NewParamsInstance;
		}

		HttpRequestParams& HttpRequestParams::AppendPayloadValue(std::string Key,
																 Modio::Detail::Buffer RawPayloadBuffer)
		{
            // No need to try to append an empty buffer
            if (RawPayloadBuffer.GetSize() == 0)
            {
                return *this;
            }
            
			auto KeyIterator = PayloadMembers.find(Key);
			if (KeyIterator != PayloadMembers.end())
			{
				PayloadMembers.erase(KeyIterator);
			}
			PayloadMembers.emplace(Key, MakePayloadContent(std::move(RawPayloadBuffer)));
			return *this;
		}

		HttpRequestParams HttpRequestParams::AppendPayloadFile(std::string Key,
															   Modio::filesystem::path PathToFileToUpload,
															   Modio::Optional<Modio::FileOffset> FileOffset,
															   Modio::Optional<Modio::FileSize> ContentSize) const
		{
			HttpRequestParams NewParamsInstance = HttpRequestParams(*this);
			Modio::Optional<PayloadContent> MaybePayload =
				MakePayloadContent(PathToFileToUpload, FileOffset, ContentSize);
			if (MaybePayload)
			{
				auto KeyIterator = NewParamsInstance.PayloadMembers.find(Key);
				if (KeyIterator != NewParamsInstance.PayloadMembers.end())
				{
					NewParamsInstance.PayloadMembers.erase(KeyIterator);
				}
				NewParamsInstance.PayloadMembers.emplace(Key, MaybePayload.value());
			}
			// Warning about null payload file will be emitted by MakePayloadContent, so we don't need to do it again
			// here
			return NewParamsInstance;
		}

		Modio::Detail::HttpRequestParams& HttpRequestParams::SetAuthTokenOverride(const std::string& AuthToken)
		{
			AuthTokenOverride = AuthToken;
			return *this;
		}

		Modio::Detail::HttpRequestParams HttpRequestParams::SetAuthTokenOverride(const std::string& AuthToken) const
		{
			auto NewParamsInstance = HttpRequestParams(*this);
			NewParamsInstance.AuthTokenOverride = AuthToken;
			return NewParamsInstance;
		}

		Modio::Detail::HttpRequestParams& HttpRequestParams::SuppressPlatformHeader()
		{
			bSuppressPlatformHeader = true;
			return *this;
		}

		Modio::Detail::HttpRequestParams HttpRequestParams::SuppressPlatformHeader() const
		{
			auto NewParamsInstance = HttpRequestParams(*this);
			NewParamsInstance.bSuppressPlatformHeader = true;
			return NewParamsInstance;
		}


		Modio::Detail::HttpRequestParams& HttpRequestParams::SetRange(Modio::FileOffset Start,
																	  Modio::Optional<Modio::FileOffset> End)
		{
			StartOffset = Start;
			EndOffset = End;
			return *this;
		}

		Modio::Detail::HttpRequestParams HttpRequestParams::SetRange(Modio::FileOffset Start,
																	 Modio::Optional<Modio::FileOffset> End) const
		{
			auto NewParamsInstance = HttpRequestParams(*this);
			NewParamsInstance.StartOffset = Start;
			NewParamsInstance.EndOffset = End;
			return NewParamsInstance;
		}

		Modio::Detail::HttpRequestParams& HttpRequestParams::SetContentRange(Modio::FileOffset Start,
																			 Modio::FileOffset End,
																			 Modio::FileOffset Total)
		{
			ContentRangeOffsets = std::make_tuple(Start, End, Total);
			return *this;
		}

		std::string HttpRequestParams::GetServerAddress() const
		{
			if (bFileDownload)
			{
				return FileDownloadServer;
			}
			else
			{
#ifdef MODIO_SERVER_SIDE_TEST
				return MODIO_SERVER_SIDE_TEST;
#else

				auto OverrideUrl = Modio::Detail::SDKSessionData::GetEnvironmentOverrideUrl();

				if (OverrideUrl.has_value())
				{
					return OverrideUrl.value();
				}

				// @todo: Break this out so that we don't need to use this class in conjunction with the SessionData
				switch (Modio::Detail::SDKSessionData::GetEnvironment())
				{
					case Environment::Live:
						return "api.mod.io";
					case Environment::Test:
						return "api.test.mod.io";
				}
				return "";
#endif
			}
		}

		std::string HttpRequestParams::GetFormattedResourcePath() const
		{
			if (bFileDownload)
			{
				return ResourcePath;
			}
			else
			{
				// @todo: We want to clean up this, this is getting messy
				std::string Filter = FilterString.value_or("");
				return std::string(GetAPIVersionString() + GetResolvedResourcePath() + "?" + Filter +
								   GetAPIKeyString());
			}
		}

		std::string HttpRequestParams::GetVerb() const
		{
			switch (CurrentOperationType)
			{
				case Verb::GET:
					return "GET";
				case Verb::HEAD:
					return "HEAD";
				case Verb::POST:
					return "POST";
				case Verb::PUT:
					return "PUT";
				case Verb::DELETE:
					return "DELETE";
			}
			return "";
		}

		Modio::Detail::Verb HttpRequestParams::GetTypedVerb() const
		{
			return CurrentOperationType;
		}

		bool HttpRequestParams::ContainsFormData() const
		{
			if (ContentType)
			{
				if (Modio::Detail::hash_64_fnv1a_const(ContentType->c_str()) == "multipart/form-data"_hash)
				{
					return true;
				}
			}
			return false;
		}

		const Modio::Optional<std::string> HttpRequestParams::GetUrlEncodedPayload() const
		{
			// Check first if ContentType has a value before accessing it.
			if (ContentType.has_value() == false)
			{
				return {};
			}

			// TODO: make this more performant by treating the mime type as a constant so we can do a early exit if we
			// are using form-data
			if (Modio::Detail::hash_64_fnv1a_const(ContentType->c_str()) != "application/x-www-form-urlencoded"_hash)
			{
				return {};
			}
			std::string PayloadString;
			for (auto& Entry : PayloadMembers)
			{
				// While performing the multipart upload with a single part of the modfile,
				// the server expects the Content-Size to be equal to the ammount of bytes
				// to be sent to the server. An empty "key" would only append the RawBuffer
				// without the "=" symbol
				if (Entry.second.RawBuffer)
				{
					if (PayloadString.length())
					{
						PayloadString += "&";
					}
					PayloadString +=
						Entry.first + "=" + std::string(Entry.second.RawBuffer->begin(), Entry.second.RawBuffer->end());
				}
				else
				{
					// Notify the caller we're rejecting the call for a url encoded payload
					Modio::Detail::Logger().Log(
						Modio::LogLevel::Error, Modio::LogCategory::Http,
						"Rejecting request for URLencoded payload because at least one payload parameter is a file {}",
						Entry.second.PathToFile.value_or("").u8string());

					return {};
				}
			}
			if (PayloadString.length())
			{
				return PayloadString;
			}
			else
			{
				return {};
			}
		}

		Modio::FileSize HttpRequestParams::GetPayloadSize() const
		{
			MODIO_PROFILE_SCOPE(HttpRequestCalcPayloadSize);
			if (PayloadMembers.size() == 0)
			{
				return Modio::FileSize(0);
			}

			if (ContentType.has_value() && ContentType.value() == "application/x-www-form-urlencoded")
			{
				// Try to find a PayloadContent that has ContentSize != 0
				// this avoids the utilization of falible "uploadMultipart-binarydata"
				// key while doing uploads
				Modio::Optional<Modio::FileSize> ContentSize = PayloadContent::PayloadContentSize(PayloadMembers);

				if (ContentSize.has_value() == true)
				{
					return ContentSize.value();
				}
				else if (GetUrlEncodedPayload().has_value() == true)
				{
					// In case the body of an HTTP request is URL encoded, it should
					// only return the value of that payload.
					return Modio::FileSize(GetUrlEncodedPayload().value().size());
				}
				else
				{
					return Modio::FileSize(0);
				}
			}
			else
			{
				const std::string BoundaryString =
					fmt::format("\r\n--{}\r\nContent-Disposition: form-data; name=\"\"\r\n\r\n", GetBoundaryHash());
				Modio::FileSize PayloadSize = Modio::FileSize(0);
				Modio::Detail::Logger Logger;
				for (auto& ContentElement : PayloadMembers)
				{
					// Length of the name
					PayloadSize += Modio::FileSize(ContentElement.first.size());
					Logger.Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
							   "Payload element name {} calculated length {}", ContentElement.first,
							   ContentElement.first.size());
					// Length of the data
					PayloadSize += ContentElement.second.FileSize;
					Logger.Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
							   "Payload element data size for {} is {}", ContentElement.first,
							   ContentElement.second.FileSize);
					if (ContentElement.second.PType == PayloadContent::PayloadType::File)
					{
						PayloadSize += Modio::FileSize(
							fmt::format("; filename=\"{}\"", ContentElement.second.PathToFile->filename().u8string())
								.size());
						Logger.Log(
							Modio::LogLevel::Trace, Modio::LogCategory::Http,
							"Payload element is file, extra string size is {}",
							fmt::format("; filename=\"{}\"", ContentElement.second.PathToFile->filename().u8string())
								.size());
					}
					// Length of the boundary string
					PayloadSize += Modio::FileSize(BoundaryString.size());
					Logger.Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "boundary string size {}",
							   BoundaryString.size());
				}
				// Add final boundary string
				const std::string FinalBoundaryString = fmt::format("\r\n--{}\r\n", GetBoundaryHash());
				PayloadSize += Modio::FileSize(FinalBoundaryString.size());
				Logger.Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "final payload size {}", PayloadSize);
				return PayloadSize;
			}
		}

		Modio::Optional<std::pair<std::string, Modio::Detail::PayloadContent>> HttpRequestParams::
			TakeNextPayloadElement()
		{
			if (PayloadMembers.size())
			{
				auto FirstElementIt = PayloadMembers.begin();
				Modio::Optional<std::pair<std::string, Modio::Detail::PayloadContent>> FirstElement =
					std::move(*FirstElementIt);
				PayloadMembers.erase(FirstElementIt);
				return FirstElement;
			}
			else
			{
				return {};
			}
		}

		Modio::Detail::HttpRequestParams::HeaderList HttpRequestParams::GetHeaders() const
		{
			MODIO_PROFILE_SCOPE(HttpRequestGetHeaders);
			HeaderList Headers = {};

			// Default headers
			if (bSuppressPlatformHeader == false)
			{
				Headers.push_back({"x-modio-platform", MODIO_TARGET_PLATFORM_HEADER});	
			}

			switch (Modio::Detail::SDKSessionData::GetPortal())
			{
				case Portal::Apple:
					Headers.push_back({"x-modio-portal", "Apple"});
					break;
				case Portal::EpicGamesStore:
					Headers.push_back({"x-modio-portal", "EGS"});
					break;
				case Portal::GOG:
					Headers.push_back({"x-modio-portal", "GOG"});
					break;
				case Portal::Google:
					Headers.push_back({"x-modio-portal", "Google"});
					break;
				case Portal::Itchio:
					Headers.push_back({"x-modio-portal", "Itchio"});
					break;
				case Portal::Nintendo:
					Headers.push_back({"x-modio-portal", "Nintendo"});
					break;
				case Portal::PSN:
					Headers.push_back({"x-modio-portal", "PSN"});
					break;
				case Portal::Steam:
					Headers.push_back({"x-modio-portal", "Steam"});
					break;
				case Portal::XboxLive:
					Headers.push_back({"x-modio-portal", "XboxLive"});
					break;
				default:
					break;
			}

			/*if (Payload || CurrentOperationType == Modio::Detail::Verb::POST ||
				CurrentOperationType == Modio::Detail::Verb::DELETE)
			{
				Headers.push_back({"Content-Type", "application/x-www-form-urlencoded"});
			}*/

			if (ContentType.has_value())
			{
				if (ContainsFormData())
				{
					Headers.push_back(
						{"Content-Type", fmt::format("{}; boundary=\"{}\"", *ContentType, GetBoundaryHash())});
				}
				else
				{
					Headers.push_back({"Content-Type", *ContentType});
				}
			}

			// Add User Agent Header
			if (UserAgentOverride.has_value())
			{
				Headers.push_back({"User-Agent", UserAgentOverride.value()});
			}
			else
			{
				Headers.push_back({"User-Agent", "Modio-SDKv2-" MODIO_COMMIT_HASH});
			}

			const Modio::Optional<std::string>& AuthToken = GetAuthToken();
			if (AuthToken)
			{
				Headers.push_back({"Authorization", fmt::format("Bearer {}", *AuthToken)});
			}

			// Header Range
			if (StartOffset.has_value() == true)
			{
				std::uintmax_t StartValue = StartOffset ? StartOffset.value() : static_cast<uintmax_t>(0);
				// In case EndOffset does not have a value, return an empty string
				std::string EndValue = EndOffset ? std::to_string(EndOffset.value()) : "";

				// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Range
				Headers.push_back({"Range", fmt::format("bytes={}-{}", StartValue, EndValue)});
			}

			// Header Content-Range needs all three values to
			if (ContentRangeOffsets.has_value() == true)
			{
				std::uintmax_t StartValue = 0;
				std::uintmax_t EndValue = 0;
				std::uintmax_t TotalValue = 0;

				std::tie(StartValue, EndValue, TotalValue) = ContentRangeOffsets.value();

				// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Range
				Headers.push_back({"Content-Range", fmt::format("bytes {}-{}/{}", StartValue, EndValue, TotalValue)});
			}

			if (OverrideLocale)
			{
				Headers.push_back({"Accept-Language", Modio::Detail::ToString(*OverrideLocale)});
			}

			// @todo: Set Content-Type: multipart/form-data for binary payload
			return Headers;
		}

		void HttpRequestParams::SetUserAgentOverride(std::string UserAgentHeader)
		{
			UserAgentOverride = UserAgentHeader;
		}

		HttpRequestParams::HttpRequestParams(std::string Server, std::string ResourcePath)
			: bFileDownload(true),
			  FileDownloadServer(Server),
			  ResourcePath(ResourcePath),
			  GameID(0),
			  ModID(0),
			  UserID(0),
			  CurrentOperationType(Modio::Detail::Verb::GET),
			  CurrentAPIVersion(Modio::Detail::APIVersion::V1)
		{}

		Modio::Detail::Buffer HttpRequestParams::GetRequestBuffer(bool bPerformURLEncoding) const
		{
			MODIO_PROFILE_SCOPE(HttpRequestBuildRawRequestBuffer);

			std::string HeaderString = fmt::format("{0} {1} HTTP/1.1\r\n", GetVerb(), GetFormattedResourcePath());
			HeaderString += fmt::format("Host: {0}\r\n", GetServerAddress());
			for (auto& CurrentHeader : GetHeaders())
			{
				HeaderString += fmt::format("{0}: {1}\r\n", CurrentHeader.first, CurrentHeader.second);
			}
			auto CurrentPayloadString = GetUrlEncodedPayload();
			if (CurrentPayloadString.has_value())
			{
				if (bPerformURLEncoding)
				{
					std::string EncodedPayload = Modio::Detail::String::URLEncode(CurrentPayloadString.value());
					HeaderString += fmt::format("content-length: {0}\r\n", EncodedPayload.size());
					HeaderString += fmt::format("\r\n{0}\r\n", EncodedPayload);
				}
				else

				{
					HeaderString += fmt::format("content-length: {0}\r\n", CurrentPayloadString->length());
					HeaderString += fmt::format("\r\n{0}\r\n", CurrentPayloadString.value());
				}
			}

			// In linux and possibly in other platforms, it is necessary to append the content-length with the size
			// of the request, that way the server keeps the connection alive and expects that amount of data to be
			// transferred over the wire.
			if (ContainsFormData() == true || PayloadContent::PayloadContentSize(PayloadMembers).has_value())
			{
				HeaderString += fmt::format("content-length: {0}\r\n", GetPayloadSize());
			}

			Modio::Detail::Buffer HeaderBuffer(HeaderString.length());
			// Use HeaderBuffer size as param for copy to prevent ever having an overrun
			std::copy(HeaderString.begin(), HeaderString.begin() + HeaderBuffer.GetSize(), HeaderBuffer.begin());
			return HeaderBuffer;
		}

		Modio::Optional<Modio::Detail::HttpRequestParams> HttpRequestParams::FileDownload(
			std::string URL, Modio::Optional<Modio::Detail::HttpRequestParams> FromRedirect)
		{
			std::string Regex = "^(http[s]?:\\/\\/)?([-a-zA-Z0-9@:%._\\+~#?&\\/"
								"=]{2,256}\\.[a-z]{2,6}\\b[-a-zA-Z0-9@:%._\\+~#?&=]*)(.+)$";
			std::regex URLPattern(Regex, std::regex::icase);
			std::smatch MatchInfo;
			if (std::regex_search(URL, MatchInfo, URLPattern) == false || MatchInfo.size() != 4)
			{
				return {};
			}

			std::string FileDownloadServer = MatchInfo[2].str();
			std::string ResourcePath = MatchInfo[3].str();

			if (FromRedirect.has_value())
			{
				auto NewParamsInstance = HttpRequestParams(FromRedirect.value());
				NewParamsInstance.bFileDownload = true;
				NewParamsInstance.FileDownloadServer = FileDownloadServer;
				NewParamsInstance.ResourcePath = ResourcePath;
				return NewParamsInstance;
			}
			else
			{
				return HttpRequestParams(FileDownloadServer, ResourcePath);
			}
		}

		std::string HttpRequestParams::GetAPIVersionString() const
		{
			switch (CurrentAPIVersion)
			{
				case APIVersion::V1:
					return "/v1";
			}
			return "";
		}

		std::string HttpRequestParams::GetAPIKeyString() const
		{
			using namespace Modio::Detail;

			return std::string("api_key=") + *SDKSessionData::CurrentAPIKey();
		}

		const Modio::Optional<std::string> HttpRequestParams::GetAuthToken() const
		{
			using namespace Modio::Detail;

			return AuthTokenOverride.has_value() ? AuthTokenOverride.value()
												 : Modio::Detail::SDKSessionData::GetAuthenticationToken().and_then(
													   &Modio::Detail::OAuthToken::GetToken);
		}

		std::string HttpRequestParams::GetResolvedResourcePath() const
		{
			using namespace Modio::Detail;

			// @todo: This does allocation, would be nice if we could avoid doing allocation or "lock-in" the
			// parameters after we have done this call first time so that we don't need to do allocations if we call
			// it again. But maybe we won't reuse a resource path, so it won't be a problem anyway
			std::string TempResourcePath = ResourcePath;
			String::ReplaceAll(TempResourcePath, "{game-id}", std::to_string(GameID));
			String::ReplaceAll(TempResourcePath, "{mod-id}", std::to_string(ModID));
			String::ReplaceAll(TempResourcePath, "{user-id}", std::to_string(UserID));

			return TempResourcePath;
		}

#ifdef MODIO_SEPARATE_COMPILATION
		HttpRequestParams InvalidParams;
#endif

		PayloadContent::PayloadContent(const PayloadContent& Other)
		{
			switch (Other.PType)
			{
				case PayloadType::Buffer:
				{
					RawBuffer = Other.RawBuffer.value().Clone();
					PathToFile = {};
					break;
				}
				case PayloadType::File:
				case PayloadType::FilePortion:
				{
					RawBuffer = {};
					PathToFile = Other.PathToFile;
					Offset = Other.Offset;
					ContentSize = Other.ContentSize;
					break;
				}
			}

			PType = Other.PType;
			FileSize = Other.FileSize;
		}

		PayloadContent::PayloadContent(Modio::Detail::Buffer InRawBuffer)
			: RawBuffer(std::move(InRawBuffer)),
			  PType(PayloadType::Buffer)
		{
			FileSize = Modio::FileSize(RawBuffer->GetSize());
		}

		PayloadContent::PayloadContent(Modio::filesystem::path PathToFile, Modio::FileSize FSize)
			: PathToFile(PathToFile)
		{
			PType = PayloadType::File;
			FileSize = FSize;
			Offset = Modio::FileOffset(0);
			ContentSize = FSize;
		}

		PayloadContent::PayloadContent(Modio::filesystem::path PathToFile, Modio::FileSize FSize,
									   Modio::FileOffset FOffset, Modio::FileSize CSize)
			: PathToFile(PathToFile)
		{
			PType = PayloadType::FilePortion;
			FileSize = FSize;
			Offset = FOffset;
			ContentSize = CSize;
		}

		Modio::Detail::PayloadContent& PayloadContent::operator=(PayloadContent&& Other)
		{
			PType = std::move(Other.PType);
			RawBuffer = std::move(Other.RawBuffer);
			PathToFile = std::move(Other.PathToFile);
			FileSize = std::move(Other.FileSize);
			Offset = std::move(Other.Offset);
			ContentSize = std::move(Other.ContentSize);
			return *this;
		}

		PayloadContent& PayloadContent::operator=(const PayloadContent& Other)
		{
			if (this == &Other)
			{
				return *this;
			}

			switch (Other.PType)
			{
				case PayloadType::Buffer:
				{
					RawBuffer = Other.RawBuffer.value().Clone();
					PathToFile = {};
				}
				MODIO_FALL_THROUGH;
				case PayloadType::File:
				case PayloadType::FilePortion:
				{
					RawBuffer = {};
					PathToFile = Other.PathToFile;
					Offset = Other.Offset;
					ContentSize = Other.ContentSize;
				}
			}

			PType = Other.PType;
			FileSize = Other.FileSize;
			return *this;
		}

		Modio::Optional<Modio::FileSize> PayloadContent::PayloadContentSize(
			std::map<std::string, PayloadContent> Members)
		{
			Modio::FileSize Result = Modio::FileSize(0);
			// Try to find a PayloadContent that has ContentSize != 0
			// this happens when performing a "uploadMultipart-binarydata" operation
			for (auto& Iterator : Members)
			{
				if (Iterator.second.ContentSize != 0)
				{
					Result += Iterator.second.ContentSize;
				}
			}

			if (Result > 0)
			{
				return Result;
			}
			else
			{
				return {};
			}
		}
	} // namespace Detail
} // namespace Modio
