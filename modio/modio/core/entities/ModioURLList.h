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

#include "modio/core/ModioSplitCompilation.h"
#include "modio/core/entities/ModioList.h"
#include <string>
#include <vector>

namespace Modio
{
	/// @docpublic
	/// @brief A URLList subclass to match Youtube urls
	class YoutubeURLList : public Modio::List<std::vector, std::string>
	{
	};

	/// @docpublic
	/// @brief A URLList subclass to match Sketchfab urls
	class SketchfabURLList : public Modio::List<std::vector, std::string>
	{
	};

} // namespace Modio
