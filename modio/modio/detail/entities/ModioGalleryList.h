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
#include "modio/detail/entities/ModioImage.h"

namespace Modio
{
	namespace Detail
	{
		class GalleryList : public Modio::List<std::vector, Modio::Detail::Image>
		{
			friend inline void from_json(const nlohmann::json& Json, Modio::Detail::GalleryList& GalleryList);
		};

		void from_json(const nlohmann::json& Json, Modio::Detail::GalleryList& GalleryList)
		{
			Detail::ParseSafe(Json, GalleryList.InternalList, "images");
		}
	} // namespace Detail
} // namespace Modio
