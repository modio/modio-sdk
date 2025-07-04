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
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "timer/TimerServiceImplementation.h"

namespace Modio
{
	namespace Detail
	{
		class TimerService : public asio::detail::service_base<TimerService>
		{
		public:
			MODIO_IMPL explicit TimerService(asio::io_context& IOService);

			using implementation_type = TimerServiceImplementation::IOObjectImplementationType;

			MODIO_IMPL void construct(implementation_type& Implementation);
			MODIO_IMPL void destroy(implementation_type& Implementation);

			void move_construct(implementation_type& Implementation, implementation_type& Other)
			{
				PlatformImplementation->MoveIOObjectImplementation(Implementation, Other);
			}

			void move_assign(implementation_type& Implementation,
							 Modio::Detail::TimerService& MODIO_UNUSED_ARGUMENT(OtherService),
							 implementation_type& Other)
			{
				PlatformImplementation->MoveIOObjectImplementation(Implementation, Other);
			}

			void converting_move_construct(implementation_type& impl, Modio::Detail::TimerService&,
										   implementation_type& other_impl)
			{
				move_construct(impl, other_impl);
			}

			void converting_move_assign(implementation_type& impl, Modio::Detail::TimerService& other_service,
										implementation_type& other_impl)
			{
				move_assign(impl, other_service, other_impl);
			}

			template<typename CompletionToken>
			void InitializeAsync(CompletionToken&& Token)
			{
				PlatformImplementation->InitializeAsync(std::forward<CompletionToken>(Token));
			}

			MODIO_IMPL void Cancel(implementation_type& Implementation);

			MODIO_IMPL void Shutdown()
			{
				PlatformImplementation->Shutdown();
			}

			template<typename CompletionTokenType>
			auto WaitAsync(implementation_type& PlatformIOObject, CompletionTokenType&& Token)
			{
				PlatformImplementation->WaitAsync(PlatformIOObject,
												  std::forward<CompletionTokenType>(Token));
			}

		private:
			std::shared_ptr<TimerServiceImplementation> PlatformImplementation {};
		};

	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioTimerService.ipp"
#endif