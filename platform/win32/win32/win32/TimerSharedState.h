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

#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/Function2Wrapper.h"
#include "modio/detail/ModioProfiling.h"
#include "timer/TimerImplementation.h"
#include <cstdio>
#include <iostream>
#include <list>
#include <memory>

class TimerSharedState : public std::enable_shared_from_this<TimerSharedState>
{
	using TimerMap = std::map<std::chrono::steady_clock::time_point, fu2::unique_function<void(Modio::ErrorCode)>>;

public:
	TimerMap PendingTimers {};

	Modio::ErrorCode InitializeTimer(std::shared_ptr<TimerImplementation> MODIO_UNUSED_ARGUMENT(ImplementationToInitialize))
	{
		return {};
	}

	template<typename CompletionToken>
	void BeginTimerInternalAsync(std::shared_ptr<TimerImplementation> TimerToStart, CompletionToken&& Token)
	{
		// insert expiry time, wait ID into sorted container
		// insert wait ID, packaged function into second container

		static uint64_t TimerCount = 0;
		char buf[255] = {0};
		snprintf(buf, 128, "Timer%Ii", TimerCount);
		std::chrono::steady_clock::duration TimerDuration = TimerToStart->GetTimerDuration();
		fu2::unique_function<void(Modio::ErrorCode)> WrappedCallback {
			[Token = std::move(Token)](Modio::ErrorCode ec) mutable {
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [Token = std::move(Token), ec]() mutable {
							   MODIO_PROFILE_SCOPE(endTimer);
							   Token(ec);
						   });
			}};
		TimerToStart->CalculatedExpiryTime = TimerDuration + std::chrono::steady_clock::now();
		PendingTimers.insert({TimerToStart->CalculatedExpiryTime, std::move(WrappedCallback)});
		Modio::Detail::ScopedProfileEvent starttimer(buf);
		// std::cout << TimerToStart->ThreadPoolTimer << "start "
		//		  << std::chrono::steady_clock::now().time_since_epoch().count() << std::endl;
		TimerCount++;
	}

	void CancelTimer(std::shared_ptr<TimerImplementation> TimerToCancel)
	{
		auto FoundTimer = PendingTimers.find(TimerToCancel->CalculatedExpiryTime);
		if (FoundTimer != PendingTimers.end())
		{
			FoundTimer->second(Modio::make_error_code(Modio::GenericError::OperationCanceled));
			PendingTimers.erase(FoundTimer);
		}
	}
	void CancelAll()
	{
		std::for_each(PendingTimers.begin(), PendingTimers.end(), [](auto& PendingTimerCallback) mutable {
			PendingTimerCallback.second(Modio::make_error_code(Modio::GenericError::OperationCanceled));
		});
		PendingTimers.clear();
	}
};