/*
 *  Copyright (C) 2024-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/ops/mod/DeleteModDependenciesOp.h"
#endif

namespace Modio
{
	namespace Detail
	{
		DeleteModDependenciesOp::DeleteModDependenciesOp(Modio::ModID ModID,
														const std::vector<Modio::ModID>& Dependencies)
			: ModID(ModID)
		{
			SubmitParams = Modio::Detail::DeleteModDependenciesRequest
								.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
								.SetModID(ModID);

			// Add each dependency to the payload
			std::size_t Index = 0;
			for (const Modio::ModID& Dependency : Dependencies)
			{
				SubmitParams = SubmitParams.AppendPayloadValue(fmt::format("dependencies[{}]", Index),
																fmt::format("{}", Dependency));
				Index++;
			}
		}
	} // namespace Detail
} // namespace Modio