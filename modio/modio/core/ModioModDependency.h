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
#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/JsonWrapper.h"

namespace Modio
{
	/// @docpublic
	/// @brief Simple struct containing simplified information about a mod which is a dependency of another mod
	struct ModDependency
	{
		Modio::ModID ModID;
		std::string ModName;
		
		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModDependency& Dependency);
	};

	/// @docpublic
	/// @brief Container for a collection of ModDependency objects
	class ModDependencyList : public PagedResult, public List<std::vector, ModDependency>
	{
		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModDependencyList& List);
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/core/ModioModDependency.ipp"
#endif