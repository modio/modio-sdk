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
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include <chrono>
#include <memory>

#include <asio/yield.hpp>
class ProcessTimersOp
{
	std::weak_ptr<TimerSharedState> SharedState;

public:
	ProcessTimersOp(std::weak_ptr<TimerSharedState> SharedState) : SharedState(SharedState) {};

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		std::shared_ptr<TimerSharedState> PinnedState = SharedState.lock();
		reenter(CoroutineState)
		{
			while (PinnedState)
			{
				if (PinnedState->PendingTimers.size())
				{
					// Check the first timer in the queue, if there's a timer and it's expiry is in the past, then
					// invoke the callback and continue
					auto CurrentTimer = PinnedState->PendingTimers.begin();
					while (CurrentTimer != PinnedState->PendingTimers.end() &&
						   CurrentTimer->first < std::chrono::steady_clock::now())
					{
						CurrentTimer->second({});
						CurrentTimer = PinnedState->PendingTimers.erase(CurrentTimer);
					}

					// process any pending timers here by posting their completion handler to the executor with an empty
					// error code
				}
				// Queue us up to run on the next tick
				yield asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));
			}
			Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
			return;
		}
	}

private:
	asio::coroutine CoroutineState;
};

template<typename CompletionTokenType>
void BeginTimerLoopAsync(std::weak_ptr<TimerSharedState> SharedState, CompletionTokenType&& OnTimerLoopEnded)
{
	asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
		ProcessTimersOp(SharedState), OnTimerLoopEnded, Modio::Detail::Services::GetGlobalContext().get_executor());
}

#include <asio/unyield.hpp>