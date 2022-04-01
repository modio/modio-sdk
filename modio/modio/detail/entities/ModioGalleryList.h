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

#include "modio/core/entities/ModioList.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/entities/ModioImage.h"

namespace Modio
{
	class GalleryList : public Modio::List<std::vector, Modio::Detail::Image>
	{
		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::GalleryList& GalleryList)
		{
			using nlohmann::from_json;
			from_json(Json, GalleryList.InternalList);
		}
		friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::GalleryList& GalleryList)
		{
			using nlohmann::to_json;
			to_json(Json, GalleryList.InternalList);
		}
		friend bool operator==(const Modio::GalleryList& A, const Modio::GalleryList& B)
		{
			if (A.InternalList.size() != B.InternalList.size())
			{
				return false;
			}
			if (A.InternalList.empty() && B.InternalList.empty())
			{
				return true;
			}
			// loop through to ensure equality even if images are stored in a different order
			int ImageMatchCount = 0;
			for (int i = 0; i < A.InternalList.size(); i++)
			{
				for (int j = 0; j < B.InternalList.size(); j++)
				{
					if (A.InternalList.at(i).Filename == B.InternalList.at(j).Filename &&
						A.InternalList.at(i).Original == B.InternalList.at(j).Original &&
						A.InternalList.at(i).Thumb320x180 == B.InternalList.at(j).Thumb320x180)
					{
						ImageMatchCount++;
					}
				}
			}
			if (ImageMatchCount == A.InternalList.size())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	};
} // namespace Modio
