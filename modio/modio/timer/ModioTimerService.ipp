/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *  
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/timer/ModioTimerService.h"
#endif

namespace Modio
{
	namespace Detail
	{
		TimerService::TimerService(asio::io_context& IOService)
			: asio::detail::service_base<TimerService>(IOService)
		{
			PlatformImplementation = std::make_shared<TimerServiceImplementation>(*this);
		}

		void TimerService::construct(implementation_type& Implementation)
		{
			Implementation = std::make_shared<TimerImplementation>();
		}

		void TimerService::destroy(implementation_type& Implementation)
		{
			Implementation.reset();
		}
		
		void TimerService::Cancel(implementation_type& Implementation) 
		{
			PlatformImplementation->Cancel(Implementation);
		}

	} // namespace Detail
}

