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

#include "modio/core/entities/ModioFileMetadata.h"
#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include <vector>

namespace Modio
{
	/// @docpublic
	/// @brief Collection of FileMetadata objects representing mod file updates
	class ModDetails : public PagedResult, public List<std::vector, FileMetadata>
	{
		friend inline void from_json(const nlohmann::json& Json, Modio::ModDetails& ModDetails);
	};

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioModDetails.ipp"
#endif
