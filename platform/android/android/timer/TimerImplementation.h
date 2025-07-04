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

#include "modio/timer/ModioTimerImplementationBase.h"
#include <chrono>
#include <ratio>

class TimerImplementation : public Modio::Detail::TimerImplementationBase
{
public:
	std::chrono::steady_clock::time_point CalculatedExpiryTime {};
};