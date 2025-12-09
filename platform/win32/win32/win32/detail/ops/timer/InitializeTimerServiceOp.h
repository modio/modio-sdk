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

#include <asio/yield.hpp>
class InitializeTimerServiceOp
{
	std::weak_ptr<TimerSharedState> SharedState {};

	public:
	InitializeTimerServiceOp(std::weak_ptr<TimerSharedState> SharedState) : SharedState(SharedState) {}

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode MODIO_UNUSED_ARGUMENT(ec) = {})
	{
		reenter(CoroState) {
			BeginTimerLoopAsync(SharedState, [](Modio::ErrorCode) {});

			Self.complete({});
			return;
		}
	}
	private:
	ModioAsio::coroutine CoroState {};
};

#include <asio/unyield.hpp>