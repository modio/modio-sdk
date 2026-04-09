/*
 *  Copyright (C) 2021-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioModCollectionEntry.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/serialization/ModioStrongIntegerSerialization.h"
//#include "modio/detail/serialization/ModioUserSubscriptionSerialization.h"

namespace Modio
{
	inline void to_json(nlohmann::json& j, const UserSubscriptionList& List)
	{
		j = nlohmann::json {Modio::Detail::Constants::JSONKeys::UserSubscriptionList, List.Get()};
	}

	inline void from_json(const nlohmann::json& j, UserSubscriptionList& List)
	{
		if (j.is_array())
		{
			using nlohmann::from_json;
			from_json(j, List.Get());
		}
	}
} // namespace Modio