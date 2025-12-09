/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioEntitlement.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioEntitlementSerialization.h"
#include "modio/http/ModioHttpParams.h"
#include <memory>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class GetAvailableUserEntitlementsOp
		{
		public:
			GetAvailableUserEntitlementsOp(Modio::Detail::HttpRequestParams Params)
			{
				LocalState = std::make_shared<Impl>();
				LocalState->Params = Params;
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(LocalState->CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						LocalState->ResponseBuffer, LocalState->Params, Modio::Detail::CachedResponse::Disallow,
						std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						Modio::Optional<Modio::EntitlementList> AvailableEntitlements =
							TryMarshalResponse<Modio::EntitlementList>(LocalState->ResponseBuffer);

						if (AvailableEntitlements.has_value() == false)
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
							return;
						}

						Self.complete({}, AvailableEntitlements);
						return;
					}
				}
			}

		private:
			struct Impl
			{
				Modio::Detail::DynamicBuffer ResponseBuffer {};
				Modio::Detail::HttpRequestParams Params {};
				ModioAsio::coroutine CoroutineState {};
			};

			Modio::StableStorage<Impl> LocalState {};
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
