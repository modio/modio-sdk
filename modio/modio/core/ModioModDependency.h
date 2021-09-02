#pragma once

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	/// @docpublic
	/// @brief Simple struct containing simplified information about a mod which is a dependency of another mod
	/// @experimental
	struct ModDependency
	{
		Modio::ModID ModID;
		std::string ModName;
		friend void from_json(const nlohmann::json& Json, Modio::ModDependency& Dependency)
		{
			Modio::Detail::ParseSafe(Json, Dependency.ModID, "mod_id");
			Modio::Detail::ParseSafe(Json, Dependency.ModName, "name");
		}
	};

	/// @docpublic
	/// @brief Container for a collection of ModDependency objects
	/// @experimental
	class ModDependencyList : public PagedResult, public List<std::vector, ModDependency>
	{
		friend void from_json(const nlohmann::json& Json, Modio::ModDependencyList& List)
		{
			from_json(Json, static_cast<Modio::PagedResult&>(List));
			Modio::Detail::ParseSafe(Json, List.InternalList, "data");
		}
	};
} // namespace Modio