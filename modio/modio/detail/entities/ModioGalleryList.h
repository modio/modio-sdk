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

#include "ModioGeneratedVariables.h"
#include "modio/core/entities/ModioList.h"
#include "modio/detail/entities/ModioImage.h"

namespace Modio
{
	/// @docpublic
	/// @brief List subclass to contain, compare and transform images
	class GalleryList : public Modio::List<std::vector, Modio::Detail::Image>
	{
		/// @docnone
		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::GalleryList& GalleryList);
		
		/// @docnone
		friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::GalleryList& GalleryList);
		
		/// @docublic
		/// @brief Comparator operator between GalleryLists, to first compare their internal list size,
		/// then the elements contained in that list
		/// @param A left side of the comparison
		/// @param B right side of the comparison
		/// @return True when both elements are equal, including their internal list. False otherwise
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
			if (A.InternalList == B.InternalList)
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

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/entities/ModioGalleryList.ipp"
#endif