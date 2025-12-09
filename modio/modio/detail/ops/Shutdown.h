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
#include "modio/detail/AsioWrapper.h"

#include <chrono>
#include <memory>

#include "modio/core/ModioStdTypes.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		/// @brief Internal operation used by ShutdownOp to run a single outstanding handler on the IO context or
		/// else time out
		class ShutdownRunOneOp
		{
		public:
			ShutdownRunOneOp(std::shared_ptr<ModioAsio::io_context> InTargetContext) : TargetContext(InTargetContext) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				TargetContext->run_one_for(std::chrono::milliseconds(1));
				Self.complete(Modio::ErrorCode {});
			}

		private:
			std::shared_ptr<ModioAsio::io_context> TargetContext;
		};
		template<typename CompletionHandlerType>
		void ShutdownRunOneAsync(std::shared_ptr<ModioAsio::io_context> OldContext,
								 CompletionHandlerType&& OnOneHandlerCalled)
		{
			return ModioAsio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
				ShutdownRunOneOp(OldContext), OnOneHandlerCalled,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

		class ShutdownOp
		{
		public:
			ShutdownOp(std::shared_ptr<ModioAsio::io_context> ContextToFlush) : ContextToFlush(ContextToFlush)
			{
				ContextToFlush->restart();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode MODIO_UNUSED_ARGUMENT(ec) = {})
			{
				reenter(CoroutineState)
				{
					while (ContextToFlush->poll_one())
					{
						yield ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));
						// If we use the op, then we get stack overflow if the shutdown takes a long time
						// yield ShutdownRunOneAsync(ContextToFlush, std::move(Self));
					}
					Self.complete(Modio::ErrorCode {});
				}
			}

		private:
			ModioAsio::coroutine CoroutineState;
			std::shared_ptr<ModioAsio::io_context> ContextToFlush;
		};

		template<typename ShutdownCompleteCallback>
		auto ShutdownAsync(std::shared_ptr<ModioAsio::io_context> Context, ShutdownCompleteCallback&& OnShutdownComplete)
		{
			return ModioAsio::async_compose<ShutdownCompleteCallback, void(Modio::ErrorCode)>(
				Modio::Detail::ShutdownOp(std::move(Context)), OnShutdownComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		};

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
