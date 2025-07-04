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
		/// @brief Unique mod id.
		Modio::ModID ModID {};
		/// @brief Name of the mod
		std::string ModName {};
		/// @brief Unix timestamp of the date the mod was registered
		std::int64_t DateAdded = 0;
		/// @brief Unix timestamp of the date the mod was updated
		std::int64_t DateUpdated = 0;
		/// @brief The level at which this dependency sits. When greater than zero (0), it means that this dependency
		/// relies on additional dependencies.
		std::uint8_t DependencyDepth = 0;
		/// @brief Media data related to the mod logo
		Modio::Detail::Logo Logo {};

		/// @brief Information about the mod's most recent public release
		Modio::Optional<Modio::FileMetadata> FileInfo {};

		/// @brief The current ModStatus on the server: Accepted, NotAccepted, or Deleted.
		Modio::ModServerSideStatus Status = Modio::ModServerSideStatus::NotAccepted;
		/// @brief The visibility status of the mod, default to Public
		Modio::ObjectVisibility Visibility = Modio::ObjectVisibility::Public;

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModDependency& Dependency);

		/// @docnone
		friend bool operator==(const ModDependency& A, const ModDependency& B)
		{
			return A.ModID == B.ModID;
		}

		/// @docnone
		friend bool operator!=(const ModDependency& A, const ModDependency& B)
		{
			return A.ModID != B.ModID;
		}
	};

	/// @docpublic
	/// @brief Container for a collection of ModDependency objects
	class ModDependencyList : public PagedResult, public List<std::vector, ModDependency>
	{
	public:
		
		ModDependencyList() : PagedResult(), List<std::vector, ModDependency>()
		{
		}
		
		/// @docpublic
		/// @brief Insert the unique contents of a ModDependencyList to the end of this list
		void AppendUnique(const ModDependencyList& Other)
		{
			for (auto& dependency : Other)
			{
				AppendUnique(dependency);
			}
		}

		/// @docpublic
		/// @brief Insert a unique ModDependency to the end of this list
		void AppendUnique(const ModDependency& ModDependencyData)
		{
			if (std::find(InternalList.begin(), InternalList.end(), ModDependencyData) == InternalList.end())
			{
				InternalList.push_back(ModDependencyData);
				if (ModDependencyData.FileInfo.has_value())
				{
					TotalFilesize += ModDependencyData.FileInfo.value().Filesize;
					TotalFilesizeUncompressed += ModDependencyData.FileInfo.value().FilesizeUncompressed;
				}
			}
		}

		/// @brief Total size of all the dependency files in bytes.
		std::uint64_t TotalFilesize = 0;
		/// @brief Total Size of the uncompressed dependency files in bytes.
		std::uint64_t TotalFilesizeUncompressed = 0;

	private:
		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModDependencyList& List);
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/core/ModioModDependency.ipp"
#endif
