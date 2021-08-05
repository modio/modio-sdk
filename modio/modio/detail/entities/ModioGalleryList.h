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
