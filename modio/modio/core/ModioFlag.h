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

namespace Modio
{


	/// @docpublic
	/// @brief Helper class to strongly type bitfield operations
	template<typename T>
	class FlagImpl
	{
	public:
		using EnumType = T;
		using StorageType = std::underlying_type_t<T>;

		/// @docnone
		constexpr FlagImpl<T>() : Internal(0)
		{
			UpdateValue();
		}

		/// @docnone
		constexpr FlagImpl<T>(EnumType InitialValue) : Internal(Convert(InitialValue))
		{
			UpdateValue();
		}

		/// @docnone
		constexpr FlagImpl<T>(StorageType InitialValue) : Internal(InitialValue)
		{
			UpdateValue();
		}

		/// @docnone
		constexpr FlagImpl<T>(const FlagImpl<T>& InitialValue) : Internal(InitialValue.Internal)
		{
			UpdateValue();
		}

		/// @docnone
		constexpr FlagImpl<T>(FlagImpl<T>&& InitialValue) : Internal(InitialValue.Internal)
		{
			InitialValue.Internal = 0;
			InitialValue.UpdateValue();
		}

		/// @docnone
		constexpr FlagImpl<T>& operator = (EnumType InitialValue)
		{
			Internal = Convert(InitialValue);
			UpdateValue();
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T>& operator=(StorageType InitialValue)
		{
			Internal = InitialValue;
			UpdateValue();
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T>& operator=(const FlagImpl<T>& InitialValue)
		{
			Internal = InitialValue.Internal;
			UpdateValue();
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T>& operator=(FlagImpl<T>&& InitialValue)
		{
			Internal = InitialValue.Internal;
			UpdateValue();
			InitialValue.Internal = 0;
			InitialValue.UpdateValue();
			return *this;
		}

		/// @docnone
		constexpr static const FlagImpl<T> Empty()
		{
			return FlagImpl<T>();
		}

		/// @docinternal
		/// @brief Transform an EnumType to its StorageType
		constexpr static StorageType Convert(const EnumType& EnumValue)
		{
			return static_cast<StorageType>(EnumValue);
		}

		/// @docinternal
		/// @brief Retrieve the StorageType inside the FlagImpl
		constexpr StorageType RawValue() const
		{
			return Internal;
		}

		/// @docnone
		constexpr FlagImpl<T> operator|(const EnumType& EnumValue) const
		{
			return FlagImpl<T>(Internal | Convert(EnumValue));
		}

		/// @docnone
		constexpr FlagImpl<T> operator|=(const EnumType& EnumValue)
		{
			Internal |= Convert(EnumValue);
			UpdateValue();
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T> operator|(const FlagImpl<T>& EnumValue) const
		{
			return FlagImpl<T>(Internal | Convert(EnumValue));
		}

		/// @docnone
		constexpr FlagImpl<T> operator|=(const FlagImpl<T>& EnumValues)
		{
			Internal = Internal | EnumValues.Internal;
			UpdateValue();
			return *this;
		}

		/// @docinternal
		constexpr FlagImpl<T>& SetFlag(const EnumType& EnumValue)
		{
			*this |= EnumValue;
			return *this;
		}

		/// @docinternal
		constexpr FlagImpl<T>& SetFlags(const FlagImpl<T>& EnumValues)
		{
			*this |= EnumValues;
			return *this;
		}

		/// @docnone
		constexpr bool operator&(const EnumType& EnumValue) const
		{
			return (Internal & Convert(EnumValue)) != 0;
		}

		/// @docnone
		constexpr FlagImpl<T> operator&=(const EnumType& EnumValue)
		{
			Internal &= Convert(EnumValue);
			UpdateValue();
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T> operator&(const FlagImpl<T>& EnumValues) const
		{
			return FlagImpl<T>(Internal & EnumValues.Internal);
		}

		/// @docnone
		constexpr FlagImpl<T> operator&=(const FlagImpl<T>& EnumValues)
		{
			Internal &= EnumValues.Internal;
			UpdateValue();
			return *this;
		}

		/// @docnone
		constexpr bool HasFlag(const EnumType& EnumValue) const
		{
			return (Internal & Convert(EnumValue)) == Convert(EnumValue);
		}

		/// @docinternal
		constexpr bool HasFlags(const FlagImpl<T>& EnumValues) const
		{
			return (*this & EnumValues) == EnumValues;
		}

		/// @docnone
		constexpr bool operator^(const EnumType& EnumValue) const
		{
			return FlagImpl<T>(Internal ^ Convert(EnumValue));
		}

		/// @docnone
		constexpr bool operator^=(const EnumType& EnumValue)
		{
			Internal ^= Convert(EnumValue);
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T> operator^(const FlagImpl<T>& EnumValue) const
		{
			return FlagImpl<T>(Internal ^ Convert(EnumValue));
		}

		/// @docnone
		constexpr FlagImpl<T> operator^=(const FlagImpl<T>& EnumValues)
		{
			Internal &= EnumValues.Internal;
			UpdateValue();
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T>& ToggleFlag(const EnumType& EnumValue)
		{
			*this ^= EnumValue;
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T>& ToggleFlags(const FlagImpl<T>& EnumValues)
		{
			*this ^= EnumValues;
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T> operator~() const
		{
			return FlagImpl<T>(~Internal);
		}

		/// @docinternal
		constexpr bool ClearFlag(EnumType EnumValue)
		{
			Internal = Internal & ~Convert(EnumValue);
			UpdateValue();
			return *this;
		}

		/// @docinternal
		constexpr bool ClearFlags(const FlagImpl<T>& EnumValues)
		{
			*this &= ~EnumValues;
			return *this;
		}

		/// @docnone
		constexpr bool operator==(const EnumType& Other) const
		{
			return Internal == Convert(Other);
		}

		/// @docnone
		constexpr bool operator==(const FlagImpl<T>& Other) const
		{
			return Internal == Other.Internal;
		}

		/// @docnone
		constexpr bool operator!=(const EnumType& Other) const
		{
			return !(*this == Other);
		}

		/// @docnone
		constexpr bool operator!=(const FlagImpl<T>& Other) const
		{
			return Internal != Other.Internal;
		}

		/// @docnone
		constexpr bool Is(std::initializer_list<EnumType> Values)
		{
			return *this == FlagImpl<T>(Values);
		}

		/// @docinternal
		constexpr FlagImpl<T>(std::initializer_list<EnumType> Values)
		{
			for (EnumType Flag : Values)
			{
				SetFlag(Flag);
			}
		}

		MODIO_DEPRECATED(2025.2, "direct access to Value has been deprecated. Migrate existing code to use operators and methods from Modio::FlagImpl<>") 
		Modio::Optional<StorageType> Value;

	private:
		constexpr void UpdateValue()
		{
			MODIO_DISABLE_WARNING_PUSH
			MODIO_DISABLE_WARNING_DEPRECATED_DECLARATIONS
			Value = Internal;
			MODIO_DISABLE_WARNING_POP
		}
		StorageType Internal = 0;
	}; // class FlagImpl

	/// @docinternal 
	/// @brief Implements operators for bitwise or, xor, and on the EnumType. The result of those 
	/// operators is a FlagsType instance. 
	#define MODIO_DEFINE_FLAG_OPERATORS(EnumType, FlagsType)                                         \
		inline constexpr FlagsType operator|(const EnumType& a, const EnumType& b)                   \
		{                                                                                            \
			return FlagsType(std::underlying_type_t<EnumType>(std::underlying_type_t<EnumType>(a) |  \
															  std::underlying_type_t<EnumType>(b))); \
		}                                                                                            \
		inline constexpr FlagsType operator&(const EnumType& a, const EnumType& b)                   \
		{                                                                                            \
			return FlagsType(std::underlying_type_t<EnumType>(std::underlying_type_t<EnumType>(a) &  \
															  std::underlying_type_t<EnumType>(b))); \
		}                                                                                            \
		inline constexpr FlagsType operator^(const EnumType& a, const EnumType& b)                   \
		{                                                                                            \
			return FlagsType(std::underlying_type_t<EnumType>(std::underlying_type_t<EnumType>(a) ^  \
															  std::underlying_type_t<EnumType>(b))); \
		}
} // namespace Modio
