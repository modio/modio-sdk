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
			ShutdownRunOneOp(std::shared_ptr<asio::io_context> InTargetContext) : TargetContext(InTargetContext) {};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				TargetContext->run_one_for(std::chrono::milliseconds(1));
				Self.complete(Modio::ErrorCode {});
			}

		private:
			std::shared_ptr<asio::io_context> TargetContext;
		};
		template<typename CompletionHandlerType>
		void ShutdownRunOneAsync(std::shared_ptr<asio::io_context> OldContext,
								 CompletionHandlerType&& OnOneHandlerCalled)
		{
			return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
				ShutdownRunOneOp(OldContext), OnOneHandlerCalled,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

		class ShutdownOp
		{
		public:
			ShutdownOp(std::shared_ptr<asio::io_context> ContextToFlush) : ContextToFlush(ContextToFlush)
			{
				ContextToFlush->restart();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{

				
				reenter(CoroutineState)
				{
					yield asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));

					while (!ContextToFlush->stopped())
					{
						ContextToFlush->run_one_for(std::chrono::milliseconds(1));
						// If we use the op, then we get stack overflow if the shutdown takes a long time
						// yield ShutdownRunOneAsync(ContextToFlush, std::move(Self));
					}
					Self.complete(Modio::ErrorCode {});
				}
			}

		private:
			asio::coroutine CoroutineState;
			std::shared_ptr<asio::io_context> ContextToFlush;
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
