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
	template<typename T>
	class FlagImpl
	{
	public:
		using EnumType = T;
		using StorageType = std::underlying_type_t<T>;
		Modio::Optional<StorageType> Value;

		constexpr FlagImpl<T>() {};

		constexpr FlagImpl<T>(EnumType InitialValue) : Value(Convert(InitialValue)) {}

		constexpr FlagImpl<T>(StorageType InitialValue) : Value(InitialValue) {};

		constexpr static const FlagImpl<T> Empty()
		{
			return FlagImpl<T>();
		}

		constexpr static StorageType Convert(const EnumType& EnumValue)
		{
			return static_cast<StorageType>(EnumValue);
		}

		constexpr StorageType RawValue()
		{
			return Value.disjunction(0).value();
		}

		constexpr FlagImpl<T> operator|(const EnumType& EnumValue) const
		{
			return FlagImpl<T>(Value.disjunction(0).value() | Convert(EnumValue));
		}

		constexpr FlagImpl<T> operator|=(const EnumType& EnumValue)
		{
			Value = Value.disjunction(0).value() | Convert(EnumValue);
			return *this;
		}

		constexpr FlagImpl<T>& SetFlag(const EnumType& EnumValue)
		{
			Value = Value.disjunction(0).value() | Convert(EnumValue);
			return *this;
		}

		constexpr bool operator&(const EnumType& EnumValue) const
		{
			return (Value.disjunction(0).value() & Convert(EnumValue)) == Convert(EnumValue);
		}

		constexpr bool HasFlag(const EnumType& EnumValue) const
		{
			return (Value.disjunction(0).value() & Convert(EnumValue)) == Convert(EnumValue);
		}

		constexpr bool operator^(const EnumType& EnumValue) const
		{
			return FlagImpl<T>(Value.disjunction(0).value() ^ Convert(EnumValue));
		}

		constexpr bool operator^=(const EnumType& EnumValue)
		{
			Value = Value.disjunction(0).value() ^ Convert(EnumValue);
			return *this;
		}

		constexpr bool ToggleFlag(const EnumType& EnumValue)
		{
			Value = Value.disjunction(0).value() ^ Convert(EnumValue);
			return *this;
		}

		constexpr FlagImpl<T> operator~() const
		{
			return FlagImpl<T>(~Value.disjunction(0).value());
		}

		constexpr bool ClearFlag(EnumType EnumValue)
		{
			Value = Value.disjunction(0).value() & ~Convert(EnumValue);
			return *this;
		}

		constexpr bool operator==(const EnumType& Other) const
		{
			return Value.has_value() && *Value == Convert(Other);
		}

		constexpr bool operator==(const FlagImpl<T>& Other) const
		{
			return (Value.has_value() && Other.Value.has_value() && *Value == *Other.Value) ||
				   (!Value.has_value() && !Other.Value.has_value());
		}

		constexpr bool operator!=(const EnumType& Other) const
		{
			return !(*this == Other);
		}

		constexpr bool operator!=(const FlagImpl<T>& Other) const
		{
			return !(*this == Other);
		}

		constexpr bool Is(std::initializer_list<EnumType> Values)
		{
			return *this == FlagImpl<T>(Values);
		}

		constexpr FlagImpl<T>(std::initializer_list<EnumType> Values)
		{
			for (EnumType Flag : Values)
			{
				SetFlag(Flag);
			}
		}
	}; // class FlagImpl
} // namespace Modio
