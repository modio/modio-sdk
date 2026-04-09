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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioEditModParams.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioModInfoSerialization.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/impl/SDKPreconditionChecks.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SubmitModChangesOp
		{
		public:

			SubmitModChangesOp(Modio::ModID Mod, Modio::EditModParams Params);

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(ResponseBuffer, EditRequestParams,
						CachedResponse::Disallow, std::move(Self));

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
					}

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}
					else
					{
						auto ModInfoData = TryMarshalResponse<Modio::ModInfo>(ResponseBuffer);
						if (ModInfoData.has_value())
						{
							Modio::Detail::SDKSessionData::InvalidateModCache(ModInfoData.value().ModId);
							Self.complete(ec, ModInfoData);
							return;
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
							return;
						}
					}
				}
			}

		private:
			ModioAsio::coroutine CoroutineState {};
			Modio::Detail::HttpRequestParams EditRequestParams {};
			Modio::Detail::DynamicBuffer ResponseBuffer {};
		};
#include <asio/unyield.hpp>

		inline auto SubmitModChangesAsync(
			Modio::ModID Mod, Modio::EditModParams Params,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)> Callback)
		{
			bool FileExists = false;

			if (Params.LogoPath.has_value() == true)
			{
				// Because LogoPath was set, we need to check file existance
				FileExists = Modio::Detail::RequireFileExists(Params.LogoPath.value(), Callback);
			}
			else
			{
				// Because LogoPath is empty, the precondition is not necessary and we need
				// to continue as normal.
				FileExists = true;
			}

			if (FileExists == true)
			{
				return ModioAsio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>,
					void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>(
						Modio::Detail::SubmitModChangesOp(Mod, Params), Callback,
						Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		}
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "SubmitModChangesOp.ipp"
#endif