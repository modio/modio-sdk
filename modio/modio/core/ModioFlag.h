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
		Modio::Optional<StorageType> Value;

		/// @docnone
		constexpr FlagImpl<T>() {}

		/// @docnone
		constexpr FlagImpl<T>(EnumType InitialValue) : Value(Convert(InitialValue)) {}

		/// @docnone
		constexpr FlagImpl<T>(StorageType InitialValue) : Value(InitialValue) {}

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
		/// @brief Retrievet the StorageType inside the FlagImpl
		constexpr StorageType RawValue() const
		{
			return Value.disjunction(static_cast<StorageType>(0)).value();
		}

		/// @docnone
		constexpr FlagImpl<T> operator|(const EnumType& EnumValue) const
		{
			return FlagImpl<T>(Value.disjunction(static_cast<StorageType>(0)).value() | Convert(EnumValue));
		}

		/// @docnone
		constexpr FlagImpl<T> operator|=(const EnumType& EnumValue)
		{
			Value = Value.disjunction(static_cast<StorageType>(0)).value() | Convert(EnumValue);
			return *this;
		}

		/// @docinternal
		constexpr FlagImpl<T>& SetFlag(const EnumType& EnumValue)
		{
			Value = Value.disjunction(static_cast<StorageType>(0)).value() | Convert(EnumValue);
			return *this;
		}

		/// @docnone
		constexpr bool operator&(const EnumType& EnumValue) const
		{
			return (Value.disjunction(static_cast<StorageType>(0)).value() & Convert(EnumValue)) == Convert(EnumValue);
		}

		/// @docnone
		constexpr bool HasFlag(const EnumType& EnumValue) const
		{
			return (Value.disjunction(static_cast<StorageType>(0)).value() & Convert(EnumValue)) == Convert(EnumValue);
		}

		/// @docnone
		constexpr bool operator^(const EnumType& EnumValue) const
		{
			return FlagImpl<T>(Value.disjunction(static_cast<StorageType>(0)).value() ^ Convert(EnumValue));
		}

		/// @docnone
		constexpr bool operator^=(const EnumType& EnumValue)
		{
			Value = Value.disjunction(static_cast<StorageType>(0)).value() ^ Convert(EnumValue);
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T>& ToggleFlag(const EnumType& EnumValue)
		{
			Value = Value.disjunction(static_cast<StorageType>(0)).value() ^ Convert(EnumValue);
			return *this;
		}

		/// @docnone
		constexpr FlagImpl<T> operator~() const
		{
			return FlagImpl<T>(~Value.disjunction(static_cast<StorageType>(0)).value());
		}

		/// @docinternal
		constexpr bool ClearFlag(EnumType EnumValue)
		{
			Value = Value.disjunction(static_cast<StorageType>(0)).value() & ~Convert(EnumValue);
			return *this;
		}

		/// @docnone
		constexpr bool operator==(const EnumType& Other) const
		{
			return Value.has_value() && *Value == Convert(Other);
		}

		/// @docnone
		constexpr bool operator==(const FlagImpl<T>& Other) const
		{
			return (Value.has_value() && Other.Value.has_value() && *Value == *Other.Value) ||
				   (!Value.has_value() && !Other.Value.has_value());
		}

		/// @docnone
		constexpr bool operator!=(const EnumType& Other) const
		{
			return !(*this == Other);
		}

		/// @docnone
		constexpr bool operator!=(const FlagImpl<T>& Other) const
		{
			return !(*this == Other);
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
	}; // class FlagImpl
} // namespace Modio
