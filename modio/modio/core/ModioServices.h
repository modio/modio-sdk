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
#include "modio/core/ModioLogEnum.h"
#include "modio/detail/AsioWrapper.h"
#include <atomic>

MODIO_DISABLE_WARNING_PUSH
MODIO_DISABLE_WARNING_DEPRECATED_DECLARATIONS

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Class intended to contain static data and accessors for the IO services the SDK uses internally
		/// May eventually contain all IO service instances
		class Services
		{
		public:
			/// @brief Singleton getter containing the global IO context for all mod.io SDK operations
			/// @return reference to the global io_context object
			static ModioAsio::io_context& GetGlobalContext()
			{
				return *(std::atomic_load(&GetGlobalContextInternal()).get());
			}

			/// @brief Restart the GlobalContext
			/// @return A new instance of the GlobalContext
			static std::shared_ptr<ModioAsio::io_context> ResetGlobalContext()
			{
				// Make a copy of the old context. Note this is not a reference, but an actual copy so lifetime is
				// extended.
				std::shared_ptr<ModioAsio::io_context> OldContext = GetGlobalContextInternal();
				std::atomic_exchange(&GetGlobalContextInternal(), std::make_shared<ModioAsio::io_context>(1));
				return OldContext;
			}

			/// @brief Static method that references the GlobalContext
			template<typename ServiceType>
			static ServiceType& GetGlobalService()
			{
				return ModioAsio::use_service<ServiceType>(GetGlobalContext());
			}

		private:

			/// @docinternal
			/// @brief Static method that stores the GlobalContext
			static std::shared_ptr<ModioAsio::io_context>& GetGlobalContextInternal()
			{
				/// @docnone
				/// @brief Singleton struct to contain a Globalcontext
				struct ContextHolder
				{
					std::shared_ptr<ModioAsio::io_context> Context;
					
					/// @docnone
					/// @brief Default constructor
					ContextHolder()
					{
						Context = std::make_shared<ModioAsio::io_context>(1);
					}
				};
				static ContextHolder InternalContext {};
				return InternalContext.Context;
			}
		};
	} // namespace Detail
} // namespace Modio

MODIO_DISABLE_WARNING_POP