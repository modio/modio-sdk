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
								   Modio::ErrorCode ec, CallbackArgs&&... Args) {
				//handle error code adaptor here			
				Callback(ec, std::forward<CallbackArgs>(Args)...);
						});
		}
	} // namespace Detail

} // namespace Modio