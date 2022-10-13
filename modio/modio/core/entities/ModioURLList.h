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
#include "modio/detail/JsonWrapper.h"
#include <string>
#include <vector>

namespace Modio
{
	/// @brief Typedef a List<vector,string> into URLList
	using URLList = List<std::vector,std::string>;

	/// @docpublic
	/// @brief A URLList subclass to match Youtube urls
	class YoutubeURLList : public URLList
	{
		/// @docnone
		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::YoutubeURLList& YoutubeURLList);
		
		/// @docnone
		friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::YoutubeURLList& YoutubeURLList);
	};

	/// @docpublic
	/// @brief A URLList subclass to match Sketchfab urls
	class SketchfabURLList : public URLList
	{
		/// @docnone
		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::SketchfabURLList& SketchfabURLList);
		
		/// @docnone
		friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::SketchfabURLList& SketchfabURLList);
	};

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioURLList.ipp"
#endif