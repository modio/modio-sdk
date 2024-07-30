/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioModDependency.h"
#include "modio/detail/serialization/ModioPagedResultSerialization.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::ModDependency& Dependency)
	{
		Modio::Detail::ParseSafe(Json, Dependency.ModID, "mod_id");
		Modio::Detail::ParseSafe(Json, Dependency.ModName, "name");


		Modio::Detail::ParseSafe(Json, Dependency.DateAdded, "date_added");
		Modio::Detail::ParseSafe(Json, Dependency.DateUpdated, "date_updated");
		Modio::Detail::ParseSafe(Json, Dependency.DependencyDepth, "dependency_depth");
		Modio::Detail::ParseSafe(Json, Dependency.Logo, "logo");

		Modio::FileMetadata FileInfo {};
		if (Detail ::ParseSafe(Json, FileInfo, "modfile"))
		{
			if (FileInfo.ModId == -1)
			{
				// This means that the FileInfo was found in
				// the list of object but did not have a parsable
				// data in the json.
				Dependency.FileInfo = {};
			}
			else
			{
				Dependency.FileInfo = FileInfo;
			}
		}

		Modio::Detail::ParseSafe(Json, Dependency.Status, "status");
		Modio::Detail::ParseSafe(Json, Dependency.Visibility, "visible");
	}

	inline void from_json(const nlohmann::json& Json, Modio::ModDependencyList& List)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(List));
		Modio::Detail::ParseSafe(Json, List.InternalList, "data");

		for (auto& dependency : List)
		{
			if (dependency.FileInfo.has_value())
			{
				List.TotalFilesize += dependency.FileInfo.value().Filesize;
				List.TotalFilesizeUncompressed += dependency.FileInfo.value().FilesizeUncompressed;
			}
		}
	}
} // namespace Modio
