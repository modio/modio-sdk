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
	/// @docpublic
	/// @brief Parameters to start a metrics session
	struct MetricsSessionStartParams
	{
	public:
		/// @brief A unique Guid to identify the starting session
		Modio::Guid SessionId;

		/// @brief Milliseconds since UNIX when this request was made
		int64_t SessionTimestamp;

		/// @brief Hashed value of the session timestamp, Id, nonce and the secret metrics key
		std::string SessionHash;

		/// @brief A unique Guid to identify this request
		Modio::Guid SessionNonce;

		/// @brief The list of mods being tracked in this session
		std::vector<Modio::ModID> ModIds;

		/// @brief An incremental order Id for each request sent to the server.
		uint64_t SessionOrderId;

		/// @docnone
		MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::MetricsSessionStartParams& Params);
	};
} // namespace Modio