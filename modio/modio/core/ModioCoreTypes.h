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
#include <cstdint>
#include <nlohmann/json.hpp>
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
		XboxLive
	};

	/// @docpublic
	/// @brief Enum representing mod logo sizes
	enum class LogoSize : std::uint8_t
	{
		Original,
		Thumb320, ///< 320x180
		Thumb640, ///< 640x360
		Thumb1280 ///< 1280x720
	};

	/// @docpublic
	/// @brief Enum representing avatar image sizes
	enum class AvatarSize : std::uint8_t
	{
		Original,
		Thumb50, ///< 50x50
		Thumb100 ///< 100x100
	};

	enum class GallerySize : std::uint8_t
	{
		Original,
		Thumb320 ///< 320x180
	};

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
		constexpr explicit StrongInteger(UnderlyingIntegerType Value) : Value(Value) {};

		constexpr explicit StrongInteger() = default;

		/// @brief Allow implicit conversion to the underlying integer type
		constexpr operator UnderlyingIntegerType() const
		{
			return Value;
		}

		/// @brief Allow us to explicit convert to our underlying type
		constexpr UnderlyingIntegerType operator*() const
		{
			return Value;
		}

		///@{
		/** Arithmetic and logical operator overloads */

		constexpr StrongInteger& operator+=(const StrongInteger Other)
		{
			Value += Other.Value;
			return *this;
		}

		constexpr StrongInteger& operator-=(const StrongInteger Other)
		{
			Value -= Other.Value;
			return *this;
		}

		constexpr StrongInteger operator+(const StrongInteger Other) const
		{
			return StrongInteger(Value + Other.Value);
		}

		constexpr StrongInteger operator-(const StrongInteger Other) const
		{
			return StrongInteger(Value - Other.Value);
		}

		constexpr bool operator>(const StrongInteger Other) const
		{
			return Value > Other.Value;
		}

		constexpr bool operator<(const StrongInteger Other) const
		{
			return Value < Other.Value;
		}
		constexpr bool operator>=(const StrongInteger Other) const
		{
			return Value >= Other.Value;
		}

		constexpr bool operator<=(const StrongInteger Other) const
		{
			return Value <= Other.Value;
		}

		constexpr bool operator==(const StrongInteger Other) const
		{
			return Value == Other.Value;
		}

		constexpr bool operator!=(const StrongInteger Other) const
		{
			return Value != Other.Value;
		}
		///@}
		friend void from_json(const nlohmann::json& Json, Modio::StrongInteger<UnderlyingIntegerType>& Integer)
		{
			using nlohmann::from_json;
			UnderlyingIntegerType RawValue;
			from_json(Json, RawValue);
			Integer = Modio::StrongInteger<UnderlyingIntegerType>(RawValue);
		}

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

		/// @brief Explicit constructor
		/// @param Code The code sent to the user's email
		explicit EmailAuthCode(const std::string& Code) : InternalCode(Code) {}
	};

	/// @docpublic
	/// @brief Strong type representing an email address
	struct EmailAddress
	{
		std::string InternalAddress;

		/// @brief Explicit constructor
		/// @param Code The code sent to the user's email
		explicit EmailAddress(const std::string& Code) : InternalAddress(Code) {}
	};

	/// @docpublic
	/// @brief Strong type representing an api key
	struct ApiKey
	{
		ApiKey() = default;

		/// @brief Explicit constructor
		/// @param Code The code sent to the user's email
		explicit ApiKey(const std::string& InApiKey) : InternalApiKey(InApiKey) {}

		bool IsValid() const
		{
			return InternalApiKey != *InvalidAPIKey();
		}

		static const ApiKey& InvalidAPIKey()
		{
			static ApiKey Invalid("");

			return Invalid;
		}

		const std::string& operator*() const
		{
			return InternalApiKey;
		}

	private:
		std::string InternalApiKey;
	};

	// Needs to be subclassed rather than type aliased so we don't accidentally provide the wrong type to a function

	/// @brief Strong type for User IDs
	struct UserID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
		UserID() : StrongInteger(-1) {};
	};

	/// @docpublic
	/// @brief Strong type for Mod IDs
	struct ModID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
		ModID() : StrongInteger(-1) {};

		constexpr bool IsValid() const
		{
			return *this != InvalidModID();
		}

		static constexpr ModID InvalidModID()
		{
			constexpr ModID ID(-1);

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
	/// @brief Strong type for Comment ID
	struct CommentID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
	};

	/// @brief Strong type for Media File ID
	struct MediaFileID : public StrongInteger<std::int64_t>
	{
		using StrongInteger::StrongInteger;
	};

	/// @brief Strong type for file offsets
	struct FileOffset : public StrongInteger<std::uintmax_t>
	{
		using StrongInteger::StrongInteger;
	};

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
		GameID() : StrongInteger(-1) {};
		
		constexpr bool IsValid() const
		{
			return *this != InvalidGameID();
		}

		static constexpr GameID InvalidGameID()
		{
			constexpr GameID ID(-1);

			return ID;
		}
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
		std::string AuthToken;
		Modio::Optional<std::string> UserEmail;
		bool bUserHasAcceptedTerms = false;
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
		Thai,
		ChineseSimplified,
		ChineseTraditional
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
		Discord
	};

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
		Info = 1, ///< Informational output containing status messages
		Warning = 2, ///< Warnings about incorrect SDK usage, timeouts
		Error = 3
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
		ModManagement
	};

	/// @docpublic
	/// @brief A rating for a mod.
	enum class Rating : int8_t
	{
		Neutral = 0,
		Positive = 1,
		Negative = -1
	};

} // namespace Modio

namespace Modio
{
	namespace Detail
	{
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
				case Language::Thai:
					return "th";
				case Language::ChineseSimplified:
					return "zh-CN";
				case Language::ChineseTraditional:
					return "zh-TW";
			}
			assert(false && "Invalid value to ToString(Modio::Language)");
			return "Unknown";
		}

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
			}

			assert(false && "Invalid value to ToString(Modio::Provider)");
			return "Unknown";
		}
	} // namespace Detail
} // namespace Modio
