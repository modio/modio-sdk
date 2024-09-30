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

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/JsonWrapper.h"
#include "modio/detail/ModioHashHelpers.h"

#include <string>

namespace Modio
{
	struct MetricsSessionEndParams
	{
	public:
		/// @brief A unique Guid to identify the active session to end
		Modio::Guid SessionId;

		/// @brief Milliseconds since UNIX when this request was made
		int64_t SessionTimestamp;

		/// @brief Hashed value of the session timestamp, Id, nonce and the secret metrics key
		std::string SessionHash;

		/// @brief unique Guid to identify this request
		Modio::Guid SessionNonce;

		/// @brief An incremental order Id for each request sent to the server.
		uint64_t SessionOrderId;

		/// @docnone
		MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::MetricsSessionEndParams& Params);
	};
} // namespace Modio