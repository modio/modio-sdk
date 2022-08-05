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
#include <functional>
#include "modio/core/ModioStdTypes.h"

namespace Modio
{
	namespace Detail
	{
		template<typename... CallbackArgs>
		auto ApplyPostAsyncChecks(std::function<void(Modio::ErrorCode, CallbackArgs...)> Callback)
		{
			return std::function<void(Modio::ErrorCode, CallbackArgs&&...)> (
			[Callback = std::forward<std::function<void(Modio::ErrorCode, CallbackArgs && ...)>>(Callback)](
								   Modio::ErrorCode ec, CallbackArgs&&... Args) 
			{
				// If the category is "system_category", log it and signal a system-wide error
				if (ec && ModioErrorCategoryID(ec.category()) == 0)
                {
                    Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::System,
                                                "System Error detected with number: {} and message: {}", ec.value(), ec.message());
                    // To avoid delivery to the end user of a cryptic system error, replace it with a "UnknownSystemError"
                    Modio::ErrorCode UpdateEC = Modio::make_error_code(Modio::SystemError::UnknownSystemError);
                    Callback(UpdateEC, std::forward<CallbackArgs>(Args)...);
                }
                else
                {
                    Callback(ec, std::forward<CallbackArgs>(Args)...);
                }
			});
		}
	} // namespace Detail

} // namespace Modio