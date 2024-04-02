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
#include "modio/detail/AsioWrapper.h"
#include "timer/TimerImplementation.h"
#include <memory>

#include <asio/yield.hpp>
class WaitForTimerOp
{
public:
	WaitForTimerOp(std::shared_ptr<TimerImplementation> Timer, std::weak_ptr<TimerSharedState> SharedState)
		: Timer(Timer),
		  SharedState(SharedState) {};
	WaitForTimerOp(WaitForTimerOp&& Other) = default;
	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		std::shared_ptr<TimerSharedState> PinnedState = SharedState.lock();
		if (!PinnedState)
		{
			Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
			return;
		}
		reenter(CoroutineState)
		{
			PinnedState->InitializeTimer(Timer);
			yield PinnedState->BeginTimerInternalAsync(Timer, std::move(Self));
			Self.complete(ec);
			return;
		}
	}

private:
	std::shared_ptr<TimerImplementation> Timer;
	asio::coroutine CoroutineState;
	std::weak_ptr<TimerSharedState> SharedState;
};
#include <asio/unyield.hpp>