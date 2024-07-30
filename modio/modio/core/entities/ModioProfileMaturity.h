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
#include "modio/core/ModioFlag.h"
#include "modio/detail/JsonWrapper.h"

namespace Modio
{
	/// @brief Maturity types classification according to the mod.io API
	///	* 0 = None set (default)
	///	* 1 = Alcohol
	///	* 2 = Drugs
	///	* 4 = Violence
	///	* 8 = Explicit
	enum class MaturityOption : std::uint8_t
	{
		None = 0,
		Alcohol = 1,
		Drugs = 2,
		Violence = 4,
		Explicit = 8
	};

	inline std::string ToString(Modio::MaturityOption option)
	{
		switch (option)
		{
			case MaturityOption::None:
				return "None";
			case MaturityOption::Alcohol:
				return "Alcohol";
			case MaturityOption::Drugs:
				return "Drugs";
			case MaturityOption::Violence:
				return "Violence";
			case MaturityOption::Explicit:
				return "Explicit";
		}

		assert(false && "Invalid value to ToString(Modio::MaturityOption)");
		return "Unknown";
	}

	/// @docpublic
	/// @brief A strong type flag object to represent MaturityOption from a mod.io info.
	/// * Maturity options flagged by the mod developer, this is only relevant if the parent game allows mods to
	/// * be labeled as mature. The value of this field will default to "Maturity Option None" unless the parent
	/// * game allows to flag mature content.
	struct ProfileMaturity : public Modio::FlagImpl<MaturityOption>
	{
		/// @docinternal
		/// @brief The default constructor would set MaturityOption to "None"
		ProfileMaturity()
		{
			Value = Convert(MaturityOption::None);
		};
		using Modio::FlagImpl<MaturityOption>::FlagImpl;

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ProfileMaturity& ProfileMaturity);

		/// @docnone
		MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::ProfileMaturity& ProfileMaturity);

	};
} // namespace Modio
