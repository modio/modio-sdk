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
