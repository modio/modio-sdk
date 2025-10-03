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
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/JsonWrapper.h"
#include <cstdint>
#include <random>
#include <sstream>
#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief Enum representing what environment your game is deployed in
	enum class Environment
	{
		Test,
		Live
	};

	/// @docnone
	/// @brief Transform the EnumValue into the underlying type
	/// @param EnumValue The enumerator value
	/// @return A statically casted value related to the enumerator
	inline auto format_as(Modio::Environment EnumValue)
	{
		return static_cast<std::underlying_type_t<Modio::Environment>>(EnumValue);
	}

	/// @docpublic
	/// @brief Enum representing the store or service your game is being distributed through
	enum class Portal
	{
		None,
		Apple,
		EpicGamesStore,
		GOG,
		Google,
		Itchio,
		Nintendo,
		PSN,
		Steam,
		XboxLive,
		Meta
	};

	/// @docnone
	/// @brief Transform the EnumValue into the underlying type
	/// @param EnumValue The enumerator value
	/// @return A statically casted value related to the enumerator
	inline auto format_as(Modio::Portal EnumValue)
	{
		return static_cast<std::underlying_type_t<Modio::Portal>>(EnumValue);
	}

	/// @docpublic
	/// @brief Enum representing the platform(s) that a modfile is enabled for
	enum class ModfilePlatform
	{
		Windows,
		Mac,
		Linux,
		Android,
		iOS,
		XboxOne,
		XboxSeriesX,
		PS4,
		PS5,
		Switch,
		Oculus,
		Source,
		WindowsServer,
		LinuxServer
	};

	/// @docnone
	/// @brief Transform the EnumValue into the underlying type
	/// @param EnumValue The enumerator value
	/// @return A statically casted value related to the enumerator
	inline auto format_as(Modio::ModfilePlatform EnumValue)
	{
		return static_cast<std::underlying_type_t<Modio::ModfilePlatform>>(EnumValue);
	}

	/// @docpublic
	/// @brief Enum representing mod logo sizes
	enum class LogoSize : std::uint8_t
	{
		Original,
		Thumb320, ///< 320x180
		Thumb640, ///< 640x360
		Thumb1280 ///< 1280x720
	};

	/// @docnone
	/// @brief Transform the EnumValue into the underlying type
	/// @param EnumValue The enumerator value
	/// @return A statically casted value related to the enumerator
	inline auto format_as(Modio::LogoSize EnumValue)
	{
		return static_cast<std::underlying_type_t<Modio::LogoSize>>(EnumValue);
	}

	/// @docpublic
	/// @brief Enum representing game icon sizes
	enum class IconSize : std::uint8_t
	{
		Original,
		Thumb64, ///< 64x64
		Thumb128, ///< 128x128
		Thumb256 ///< 256x256
	};

	/// @docnone
	/// @brief Transform the EnumValue into the underlying type
	/// @param EnumValue The enumerator value
	/// @return A statically casted value related to the enumerator
	inline auto format_as(Modio::IconSize EnumValue)
	{
		return static_cast<std::underlying_type_t<Modio::IconSize>>(EnumValue);
	}

	/// @docpublic
	/// @brief Enum representing avatar image sizes
	enum class AvatarSize : std::uint8_t
	{
		Original,
		Thumb50, ///< 50x50
		Thumb100 ///< 100x100
	};

	/// @docnone
	/// @brief Transform the EnumValue into the underlying type
	/// @param EnumValue The enumerator value
	/// @return A statically casted value related to the enumerator
	inline auto format_as(Modio::AvatarSize EnumValue)
	{
		return static_cast<std::underlying_type_t<Modio::AvatarSize>>(EnumValue);
	}

	/// @docpublic
	/// @brief Enum representing an image gallery size
	enum class GallerySize : std::uint8_t
	{
		Original,
		Thumb320, ///< 320x180
		Thumb1280 ///< 1280x720
	};

	/// @docnone
	/// @brief Transform the EnumValue into the underlying type
	/// @param EnumValue The enumerator value
	/// @return A statically casted value related to the enumerator
	inline auto format_as(Modio::GallerySize EnumValue)
	{
		return static_cast<std::underlying_type_t<Modio::GallerySize>>(EnumValue);
	}

	/// @docpublic
	/// @brief Strong integer type to prevent accidental function parameter reordering and prevent unwanted implicit
	/// conversions
	/// @tparam UnderlyingIntegerType The underlying integer type the StrongInteger represents
	template<typename UnderlyingIntegerType>
	struct StrongInteger
	{
	private:
		UnderlyingIntegerType Value = 0;

	public:
		using UnderlyingType = UnderlyingIntegerType;
		// Force explicit use of the constructor when we need a StrongInteger (ie this means you must construct a
		// StrongInteger to pass into a function)

		/// @docpublic
		/// @brief explicit constructor from a value of the underlying integer type
		/// @param Value initial value
		constexpr explicit StrongInteger(UnderlyingIntegerType Value) : Value(Value) {}

		/// @docnone
		constexpr explicit StrongInteger() = default;

		/// @brief Allow implicit conversion to the underlying integer type
		constexpr operator UnderlyingIntegerType() const
		{
			return Value;
		}

		/// @docnone
		/// @brief Allow us to explicit convert to our underlying type
		constexpr UnderlyingIntegerType operator*() const
		{
			return Value;
		}

		///@{
		/** Arithmetic and logical operator overloads */

		/// @docnone
		constexpr StrongInteger& operator+=(const StrongInteger Other)
		{
			Value += Other.Value;
			return *this;
		}

		/// @docnone
		constexpr StrongInteger& operator-=(const StrongInteger Other)
		{
			Value -= Other.Value;
			return *this;
		}

		/// @docnone
		constexpr StrongInteger operator+(const StrongInteger Other) const
		{
			return StrongInteger(Value + Other.Value);
		}

		/// @docnone
		constexpr StrongInteger operator-(const StrongInteger Other) const
		{
			return StrongInteger(Value - Other.Value);
		}

		/// @docnone
		constexpr bool operator>(const StrongInteger Other) const
		{
			return Value > Other.Value;
		}

		/// @docnone
		constexpr bool operator<(const StrongInteger Other) const
		{
			return Value < Other.Value;
		}

		/// @docnone
		constexpr bool operator>=(const StrongInteger Other) const
		{
			return Value >= Other.Value;
		}

		/// @docnone
		constexpr bool operator<=(const StrongInteger Other) const
		{
			return Value <= Other.Value;
		}

		/// @docnone
		constexpr bool operator==(const StrongInteger Other) const
		{
			return Value == Other.Value;
		}

		/// @docnone
		constexpr bool operator!=(const StrongInteger Other) const
		{
			return Value != Other.Value;
		}
		///@}

		/// @docnone
		friend void from_json(const nlohmann::json& Json, Modio::StrongInteger<UnderlyingIntegerType>& Integer)
		{
			using nlohmann::from_json;
			UnderlyingIntegerType RawValue;
			from_json(Json, RawValue);
			Integer = Modio::StrongInteger<UnderlyingIntegerType>(RawValue);
		}

		/// @docnone
		friend void to_json(nlohmann::json& Json, const Modio::StrongInteger<UnderlyingIntegerType>& Integer)
		{
			using nlohmann::to_json;
			UnderlyingIntegerType RawValue = Integer;
			to_json(Json, RawValue);
		}
	};

	/// @docpublic
	/// @brief Strong type representing an email authentication code sent to a user email address
	struct EmailAuthCode
	{
		std::string InternalCode;

		/// @docpublic
		/// @brief Explicit constructor
		/// @param Code The code sent to the user's email
		explicit EmailAuthCode(const std::string& Code) : InternalCode(Code) {}
	};

	/// @docpublic
	/// @brief Strong type representing an email address
	struct EmailAddress
	{
		std::string InternalAddress;

		/// @docpublic
		/// @brief Explicit constructor
		/// @param Code The code sent to the user's email
		explicit EmailAddress(const std::string& Code) : InternalAddress(Code) {}
	};

	/// @docpublic
	/// @brief Strong type representing an api key
	struct ApiKey
	{
		/// @docinternal
		/// @brief Default constructor
		ApiKey() = default;

		/// @docpublic
		/// @brief Explicit constructor
		/// @param InApiKey The code sent to the user's email
		explicit ApiKey(const std::string& InApiKey) : InternalApiKey(InApiKey) {}

		/// @docinternal
		/// @brief Compare the ApiKey to an empty string
		bool IsValid() const
		{
			return InternalApiKey != *InvalidAPIKey();
		}

		/// @docinternal
		/// @brief Static function to an invalid API key
		static const ApiKey& InvalidAPIKey()
		{
			static ApiKey Invalid("");

			return Invalid;
		}

		/// @docnone
		const std::string& operator*() const
		{
			return InternalApiKey;
		}

	private:
		std::string InternalApiKey {};
	};

	// Needs to be subclassed rather than type aliased so we don't accidentally provide the wrong type to a function

	/// @docpublic
	/// @brief Strong type for User IDs
	struct UserID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
		/// @docinternal
		/// @brief Default constructor
		UserID() : StrongInteger(-1) {}

		/// @docinternal
		/// @brief Compare the UserID to an invalid instance
		constexpr bool IsValid() const
		{
			return *this != InvalidUserID();
		}

		/// @docinternal
		/// @brief Static function to an invalid UserID
		static constexpr UserID InvalidUserID()
		{
			constexpr UserID ID(-1);

			return ID;
		}
	};

	/// @docpublic
	/// @brief Strong type for Mod IDs
	struct ModID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
		/// @docinternal
		/// @brief Default constructor
		ModID() : StrongInteger(-1) {}

		/// @docinternal
		/// @brief Compare the ModID to an invalid instance
		constexpr bool IsValid() const
		{
			return *this != InvalidModID();
		}

		/// @docinternal
		/// @brief Static function to an invalid ModID
		static constexpr ModID InvalidModID()
		{
			constexpr ModID ID(-1);

			return ID;
		}
	};

	/// @docpublic
	/// @brief Strong type for Mod Collection IDs
	struct ModCollectionID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;

		/// @docinternal
		/// @brief Default constructor
		ModCollectionID() : StrongInteger(-1) {}

		/// @docinternal
		/// @brief Compare the ModCollectionID to an invalid instance
		constexpr bool IsValid() const
		{
			return *this != InvalidCollectionID();
		}

		/// @docinternal
		/// @brief Static function to an invalid ModCollectionID
		static constexpr ModCollectionID InvalidCollectionID()
		{
			constexpr ModCollectionID ID(-1);
			return ID;
		}
	};

	/// @docpublic
	/// @brief Strong type for File Metadata IDs
	struct FileMetadataID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
	};

	/// @docpublic
	/// @brief Strong type for Media File ID
	struct MediaFileID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
	};

	/// @docpublic
	/// @brief Strong type for file offsets
	struct FileOffset : public StrongInteger<std::uintmax_t>
	{
		using StrongInteger::StrongInteger;
	};

	/// @docpublic
	/// @brief Strong type for file sizes
	struct FileSize : public StrongInteger<std::uintmax_t>
	{
		using StrongInteger::StrongInteger;
	};

	/// @docpublic
	/// @brief Strong type for a game ID
	struct GameID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
		/// @docinternal
		/// @brief Default constructor
		GameID() : StrongInteger(-1) {}

		/// @docinternal
		/// @brief Compare the GameID to an invalid instance
		constexpr bool IsValid() const
		{
			return *this != InvalidGameID();
		}

		/// @docinternal
		/// @brief Static function to an invalid GameID
		static constexpr GameID InvalidGameID()
		{
			constexpr GameID ID(-1);

			return ID;
		}
	};

	/// @docpublic
	/// @brief Strong type representing a Guid as 16byte binary value
	struct GuidV4
	{
		/// @docinternal
		GuidV4()
		{
			Raw64[1] = Raw64[0] = 0;
		}

		/// @docinternal
		GuidV4(const uint8_t UUID[16])
		{
			std::memmove(Raw8, UUID, 16);
			// assert(IsValid());
		}

		/// @docinternal
		GuidV4(const GuidV4& UUID)
		{
			std::memmove(Raw8, UUID.Raw8, 16);
			// assert(IsValid());
		}

		/// @docinternal
		GuidV4(const std::string& UUID)
		{
			Parse(UUID);
		}

		/// @docinternal
		GuidV4& operator=(const GuidV4& UUID)
		{
			std::memcpy(Raw8, UUID.Raw8, 16);
			// assert(IsValid());
			return *this;
		}

		/// @docinternal
		GuidV4& operator=(const std::string& UUID)
		{
			Parse(UUID);
			return *this;
		}

		void ConvertByteOrder()
		{
			std::swap(Raw8[0], Raw8[3]);
			std::swap(Raw8[1], Raw8[2]);
			std::swap(Raw8[4], Raw8[5]);
			std::swap(Raw8[6], Raw8[7]);
		}

		/// @brief Given a GUID of the form: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX,
		/// convert into the binary representation
		/// @param Input
		/// @return
		bool Parse(const std::string& Input)
		{
			FromString(Input);
			return IsValid();
		}

		/// @docinternal
		MODIO_IMPL void Generate();

		/// @docinternal
		/// @brief Static function to generate an invalid Guid
		static GuidV4 InvalidGuid()
		{
			return GuidV4();
		}

		/// @docinternal
		/// @brief Ensure the GUID value conforms to a randomised version 4 GUID.
		bool IsValid() const
		{
			// ensure we have a valid Version 4 UUID, variant 2.
			return (Raw8[7] & 0xF0) == 0x40 && (Raw8[8] & 0xC0) == 0x80;
		}

		/// @brief converts from a binary UUID to a text string UUID
		/// @return the UUID as a text string
		MODIO_IMPL std::string ToString() const;

		/// @brief converts from a text string UUID to a binary UUID
		/// @param String the UUID as a text string
		MODIO_IMPL void FromString(const std::string& String);

		/// @brief Test for equality
		/// @param Other the UUID to compare against
		/// @return true if the GUIDs are equal
		bool operator==(const GuidV4& Other) const
		{
			return Raw64[0] == Other.Raw64[0] && Raw64[1] == Other.Raw64[1];
		}

		/// @brief test for inequality
		/// @param Other the UUID to compare against
		/// @return true if the GUIDs are equal
		bool operator!=(const GuidV4& Other) const
		{
			return !(*this == Other);
		}

		/// @brief less that comparison for sorting
		/// @param Other the UUID to compare against
		/// @return true if this UUID is less than Other
		bool operator<(const GuidV4& Other) const
		{
			if (Raw64[0] == Other.Raw64[0])
				return Raw64[1] < Other.Raw64[1];
			return Raw64[0] < Other.Raw64[0];
		}

		/// @brief greater that comparison for sorting
		/// @param Other
		/// @return true if this UUID is greater than Other
		bool operator>(const GuidV4& Other) const
		{
			if (Raw64[0] == Other.Raw64[0])
				return Raw64[1] > Other.Raw64[1];
			return Raw64[0] > Other.Raw64[0];
		}

		/// @brief returns the raw bytes. Note, internally this is stored in MS binary format
		/// @return pointer to the raw bytes in the GUID
		uint8_t* Bytes()
		{
			return Raw8;
		}

		/// @brief returns the raw bytes. Note, internally this is stored in MS binary format
		/// @return pointer to the raw bytes in the GUID
		const uint8_t* Bytes() const
		{
			return Raw8;
		}

	private:
		union
		{
			uint8_t Raw8[16];
			uint64_t Raw64[2];
		};
	};

	/// @docpublic
	/// @brief Strong type representing a Guid
	struct Guid
	{
		/// @docinternal
		/// @brief Default constructor
		Guid() = default;

		/// @docpublic
		/// @brief Generates a new Guid.
		/// NOTE: This is a minimalistic implementation of generating a Guid for metrics, an improved more robust
		/// per-platform implementation is planned
		MODIO_IMPL static Guid GenerateGuid();

		/// @docpublic
		/// @brief Explicit constructor
		/// @param InGuid string based Guid
		explicit Guid(const std::string& InGuid) : InternalGuid(InGuid) {}

		/// @docpublic
		/// @brief Explicit constructor
		/// @param InGuid raw 128bit Guid
		MODIO_IMPL explicit Guid(const Modio::GuidV4& InGuid);

		/// @docinternal
		/// @brief Compare the Guid to an empty string
		bool IsValid() const
		{
			return InternalGuid != *InvalidGuid();
		}

		/// @docinternal
		/// @brief Static function to an invalid Guid
		static Guid InvalidGuid()
		{
			return Guid();
		}

		/// @docnone
		const std::string& operator*() const
		{
			return InternalGuid;
		}

	private:
		std::string InternalGuid;
	};

	/// @docpublic
	/// @brief Strong type wrapping a handle to an as-yet-uncreated mod
	struct ModCreationHandle : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
	};
	// TODO: @Modio-core strong type for filesystem::paths that are directories?

	/// @docpublic
	/// @brief Simple struct to encapsulate data passed to external authentication systems
	struct AuthenticationParams
	{
		/// @brief AuthToken String received by an authentication provider, that later is used
		/// to verify a user on the mod.io API
		std::string AuthToken;
		/// @brief UserEmail Optional string that associates an email with a user
		Modio::Optional<std::string> UserEmail;
		/// @brief bUserHasAcceptedTerms Boolean value that signals when a user has accepted the
		/// mod.io Terms of Service. True when they are accepted, false otherwise.
		bool bUserHasAcceptedTerms = false;
		/// @brief bURLEncodeAuthToken Boolean value to let the SDK know if the AuthToken requires
		/// URL encoding before sending the string to the mod.io API. By default, it would not
		/// perform URL encoding.
		bool bURLEncodeAuthToken = false;
		/// @brief ExtendedParameters A map to store extended parameters required by some providers.
		std::map<std::string, std::string> ExtendedParameters;
	};

	/// @docpublic
	/// @brief Enum representing the languages mod.io support responses in
	enum class Language
	{
		English,
		Bulgarian,
		French,
		German,
		Italian,
		Polish,
		Portuguese,
		Hungarian,
		Japanese,
		Korean,
		Russian,
		Spanish,
		SpanishLatinAmerican,
		Thai,
		ChineseSimplified,
		ChineseTraditional,
		Turkish,
		Ukrainian,
		Arabic
	};

	/// @docpublic
	/// @brief Simple struct to encapsulate data passed to external authentication systems
	enum class AuthenticationProvider
	{
		XboxLive,
		Steam,
		GoG,
		Itch,
		Switch,
		Discord,
		PSN,
		Oculus,
		Epic,
		OpenID,
		Apple,
		GoogleIDToken,
		GoogleServerSideToken
	};

	/// @docpublic
	/// @brief Simple struct representing a validation error from the REST API
	struct FieldError
	{
		/// @brief String description of the field failing validation
		std::string Field;
		/// @brief String containing the reason for the validation failure
		std::string Error;
	};

	/// @docpublic
	/// @brief Degree of severity for the log output
	enum class LogLevel
	{
		Trace = 0, ///< Detailed low-level debugging output. Not intended for general use
		Detailed = 1, ///< Detailed but not low-level. Generally useful for some mid-level information for debugging.
		Info = 2, ///< Informational output containing status messages
		Warning = 3, ///< Warnings about incorrect SDK usage, timeouts
		Error = 4
	};

	/// @docpublic
	/// @brief The category of the log message (usually the internal service the message is originating from)
	enum class LogCategory
	{
		Core,
		File,
		Http,
		Compression,
		User,
		ModManagement,
		Test,
		System,
		ModMetrics
	};

	/// @docpublic
	/// @brief A rating for a mod.
	enum class Rating : int8_t
	{
		Neutral = 0,
		Positive = 1,
		Negative = -1
	};

	/// @docpublic
	/// @brief The visibility of a mod.
	enum class ObjectVisibility : int8_t
	{
		Hidden = 0,
		Public = 1
	};

	///	@brief File approval status to filter by. Passed in ExtendedInitializationParameters for the duration of a
	/// session.
	enum class PlatformStatus
	{
		LiveAndPending,
		PendingOnly,
		ApprovedOnly
	};

	/// @docpublic
	/// @brief Simple struct to encapsulate portal-specific data required when consuming entitlements
	struct EntitlementParams
	{
		/// @brief ExtendedParameters A map to store extended parameters required by some portals.
		std::map<std::string, std::string> ExtendedParameters;
	};

	/// @docpublic
	///	@brief Type of entitlement
	enum class EntitlementType
	{
		VirtualCurrency = 0,
		SKU = 1
	};

	/// @docpublic
	/// @brief Simple struct to store metric session specific parameters for use in
	/// [`Modio::MetricsSessionStartAsync`](#metricssessionstartasync)
	struct MetricsSessionParams
	{
		/// @brief Set a custom Session Id to be used in the metrics service for your session
		Modio::Optional<Modio::Guid> SessionId;

		/// @brief Includes mods which will be tracked as part of the metrics service
		std::vector<Modio::ModID> ModIds;
	};

	/// @docpublic
	/// @brief Where the storage has been written to
	enum class StorageLocation : uint8_t
	{
		// Locally installed mods for this game, including temp mods and those installed by other users on this system
		Local = 0,
		// Cached image data such as mod logos and gallery images
		Cache = 1
	};

	/// @docpublic
	/// @brief The type of storage usage
	enum class StorageUsage : uint8_t
	{
		Consumed = 0,
		Available = 1
	};

	/// @docpublic
	/// @brief Structure containing storage usage information including availability and consumption
	struct StorageInfo
	{
		/// @docpublic
		/// @brief Returns the recorded storage space for a given storage location and usage type
		/// @param Location Where the storage has been written to
		/// @param Usage The type of storage usage recorded
		/// @return Modio::FileSize for total storage space in bytes
		Modio::FileSize GetSpace(Modio::StorageLocation Location, Modio::StorageUsage Usage) const
		{
			const auto It = StorageData.find({Location, Usage});
			if (It != StorageData.end())
			{
				return It->second;
			}

			return Modio::FileSize(0);
		}

		/// @docinternal
		/// @brief Default constructor
		StorageInfo() {}

		/// @docpublic
		/// @brief Get the storage quota for a given `Modio::StorageLocation` if one has been set.
		/// @param Location The type of storage to get the quota for.
		/// @return `Modio::Optional<Modio::FileSize>` of the storage quota in bytes for the specified
		/// `Modio::StorageLocation`. Empty if no quota is set.
		static MODIO_IMPL Modio::Optional<Modio::FileSize> GetQuota(Modio::StorageLocation Location);

	private:
		/// @docnone
		std::map<std::tuple<Modio::StorageLocation, Modio::StorageUsage>, Modio::FileSize> StorageData;

		/// @docnone
		friend MODIO_IMPL void SetSpace(Modio::StorageInfo& Info, Modio::StorageLocation Location,
										Modio::StorageUsage Usage, Modio::FileSize Space);
	};
} // namespace Modio

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief The read/write access of a file
		enum class FileMode
		{
			ReadOnly,
			ReadWrite
		};

		/// @docnone
		///	@brief Transform the PlatformStatus to its query parameter string for appending to requests
		inline std::string ToString(Modio::PlatformStatus Status)
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

		/// @docnone
		/// @brief Transform a Language to its two letter string representation
		inline std::string ToString(Modio::Language Locale)
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
			}
			assert(false && "Invalid value to ToString(Modio::Language)");
			return "Unknown";
		}

		/// @docinternal
		/// @brief Transform an Authentication Provider to its string representation which
		/// match the backend list of supported platforms
		inline std::string ToString(Modio::AuthenticationProvider Provider)
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

			assert(false && "Invalid value to ToString(Modio::Provider)");
			return "Unknown";
		}
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/core/ModioCoreTypes.ipp"
#endif