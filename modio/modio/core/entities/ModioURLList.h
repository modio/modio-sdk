#pragma once
#include "ModioGeneratedVariables.h"
#include "modio/core/entities/ModioList.h"
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

namespace Modio
{
	using URLList = List<std::vector,std::string>;

	class YoutubeURLList : public URLList
	{
		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::YoutubeURLList& YoutubeURLList);
		friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::YoutubeURLList& YoutubeURLList);
	};

	class SketchfabURLList : public URLList
	{
		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::SketchfabURLList& SketchfabURLList);
		friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::SketchfabURLList& SketchfabURLList);
	};

}


#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioURLList.ipp"
#endif