/*
 *  Copyright (C) mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioMetricsSessionHeartbeatParams.h"

namespace Modio
{
	inline void to_json(nlohmann::json& Json, const Modio::MetricsSessionHeartbeatParams& Params)
	{
		Json = nlohmann::json {{"session_id", *Params.SessionId},
							   {"session_ts", Params.SessionTimestamp},
							   {"session_hash", Params.SessionHash},
							   {"session_nonce", *Params.SessionNonce},
							   {"session_order_id", Params.SessionOrderId}};
	}
} // namespace Modio