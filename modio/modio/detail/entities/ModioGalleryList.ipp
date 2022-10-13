/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *  
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/entities/ModioGalleryList.h"
#endif

#include "modio/detail/ModioJsonHelpers.h"


namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::GalleryList& GalleryList)
	{
		using nlohmann::from_json;
		from_json(Json, GalleryList.InternalList);
	}
	void to_json(nlohmann::json& Json, const Modio::GalleryList& GalleryList)
	{
		using nlohmann::to_json;
		to_json(Json, GalleryList.InternalList);
	}
}