/*
 *  Copyright (C) 2021-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/ModManagementLoop.h"

namespace Modio
{
	Modio::ErrorCode EnableModManagement(std::function<void(Modio::ModManagementEvent)> ModManagementHandler)
	{
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();

			// TODO: double check if there's an easy way for us to safely read these variables so we can immediately
			// return a value
			if (!Modio::Detail::SDKSessionData::IsInitialized())
			{
				Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::Core,
											"SDK is not initialized. Cannot Enable Mod Management.");
				return Modio::make_error_code(Modio::GenericError::SDKNotInitialized);
			}
			if (Modio::Detail::SDKSessionData::IsModManagementEnabled())
			{
				Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::Core, "Mod Management is already enabled");
				return Modio::make_error_code(Modio::ModManagementError::ModManagementAlreadyEnabled);
			}
			Modio::Detail::SDKSessionData::SetUserModManagementCallback(ModManagementHandler);
			Modio::Detail::SDKSessionData::AllowModManagement();
		}

		Modio::Detail::SDKSessionData::EnqueueTask([]() {
			Modio::Detail::BeginModManagementLoopAsync([](Modio::ErrorCode ec) mutable {
				if (ec)
				{
					Modio::Detail::Logger().Log(LogLevel::Info, Modio::LogCategory::Core,
												"Mod Management Loop halted: status message {}", ec.message());
				}
			});
		});

		Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::ModManagement, "Enabled mod management.");

		return {};
	}
} // namespace Modio
