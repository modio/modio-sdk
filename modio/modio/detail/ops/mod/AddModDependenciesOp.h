/*
 *  Copyright (C) 2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class AddModDependenciesOp
		{
		public:
			AddModDependenciesOp(Modio::ModID ModID, const std::vector<Modio::ModID>& Dependencies);

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, SubmitParams, Modio::Detail::CachedResponse::Disallow, std::move(Self));

					// Invalidate cache for mod
					Modio::Detail::SDKSessionData::InvalidateModCache(ModID);

					Self.complete(ec);
					return;
				}
			}

		private:
			Modio::ModID ModID {};
			Modio::Detail::HttpRequestParams SubmitParams {};
			ModioAsio::coroutine CoroutineState {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
		};
#include <asio/unyield.hpp>

		template<typename AddDependenciesCompleteCallback>
		void AddModDependenciesAsync(Modio::ModID ModID, const std::vector<Modio::ModID>& Dependencies,
									 AddDependenciesCompleteCallback&& OnAddDependenciesComplete)
		{
			return ModioAsio::async_compose<AddDependenciesCompleteCallback, void(Modio::ErrorCode)>(
				Modio::Detail::AddModDependenciesOp(ModID, Dependencies), OnAddDependenciesComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "AddModDependenciesOp.ipp"
#endif