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

#include "android/TimerSharedState.h"
#include "android/detail/ops/timer/InitializeTimerServiceOp.h"
#include "android/detail/ops/timer/ProcessTimersOp.h"
#include "android/detail/ops/timer/WaitForTimerOp.h"
#include "modio/detail/timer/ITimerServiceImplementation.h"
#include "timer/TimerImplementation.h"
#include <memory>

#include "modio/detail/AsioWrapper.h"
namespace Modio
{
	namespace Detail
	{
		class TimerServiceImplementation : public Modio::Detail::ITimerServiceImplementation
		{
			std::shared_ptr<TimerSharedState> SharedState;
			asio::io_context::service& OwningService;

		public:
			using IOObjectImplementationType = std::shared_ptr<TimerImplementation>;

			TimerServiceImplementation(asio::io_context::service& OwningService) : OwningService(OwningService)
			{
				SharedState = std::make_shared<TimerSharedState>();
			}

			void MoveIOObjectImplementation(IOObjectImplementationType& Implementation,
											IOObjectImplementationType& OtherImplementation)
			{
				Implementation = std::move(OtherImplementation);
			}

			template<typename CompletionTokenType>
			void InitializeAsync(CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
					InitializeTimerServiceOp(SharedState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionToken>
			auto WaitAsync(IOObjectImplementationType PlatformIOObjectInstance, CompletionToken&& Token)
			{
				return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
					WaitForTimerOp(PlatformIOObjectInstance, SharedState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
			void Cancel(IOObjectImplementationType PlatformIOObjectInstance)
			{
				if (SharedState)
				{
					SharedState->CancelTimer(PlatformIOObjectInstance);
				}
			}
			void Shutdown() override
			{
				if (SharedState)
				{
					SharedState->CancelAll();
				}
				SharedState.reset();
			};
		};
	} // namespace Detail
} // namespace Modio