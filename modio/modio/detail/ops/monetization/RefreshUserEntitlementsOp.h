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

#include "modio/core/entities/ModioEntitlementConsumptionStatusList.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/ops/userdata/VerifyUserAuthenticationOp.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/userdata/ModioUserDataService.h"
#include <memory>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class RefreshUserEntitlementsOp
		{
		public:
			RefreshUserEntitlementsOp(Modio::Detail::HttpRequestParams Params)
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
						Modio::Optional<Modio::EntitlementConsumptionStatusList> ListOpt =
							TryMarshalResponse<Modio::EntitlementConsumptionStatusList>(LocalState->ResponseBuffer);

						if (ListOpt.has_value() == false)
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
							return;
						}

						Modio::Optional<Modio::EntitlementConsumptionStatusList> ListRetry =
							ListOpt.value().EntitlementsThatRequireRetry();
						if (ListRetry.has_value() == true && ListRetry.value().Size() > 0)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Http,
														"Some entitlements needs retry");
							Self.complete(Modio::make_error_code(Modio::MonetizationError::RetryEntitlements), ListOpt);
							return;
						}

						Self.complete({}, ListOpt);
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
