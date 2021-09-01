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
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include <atomic>
#include <memory>

namespace Modio
{
	namespace Detail
	{
		class OperationQueue : public std::enable_shared_from_this<OperationQueue>
		{
			std::atomic<bool> OperationInProgress;
			std::atomic<std::int32_t> NumWaiters;
			asio::steady_timer QueueImpl;

		public:
			OperationQueue(asio::io_context& OwningContext)
				: OperationInProgress(false),
				  NumWaiters(0) ,
				  QueueImpl(OwningContext, std::chrono::steady_clock::time_point::max())
			{}
			OperationQueue(const OperationQueue& Other) = delete;

			class Ticket
			{
				struct TicketImpl
				{
					std::weak_ptr<Modio::Detail::OperationQueue> AssociatedQueue;
					bool OperationQueued = false;

				public:
					TicketImpl(std::weak_ptr<Modio::Detail::OperationQueue> AssociatedQueue)
						: AssociatedQueue(AssociatedQueue)
					{}
				};

				// Stable storage so the Ticket can have copy semantics but a stable address internally
				std::unique_ptr<TicketImpl> Impl;

			public:
				Ticket(std::weak_ptr<Modio::Detail::OperationQueue> AssociatedQueue)
				{
					Impl = std::make_unique<TicketImpl>(AssociatedQueue);
				}

				Ticket(const Ticket& Other) = delete;

				Ticket(Ticket&& Other)
				{
					Impl = std::move(Other.Impl);
				}

				~Ticket()
				{
					// Need to check here to account for being moved-from
					if (Impl)
					{
						std::shared_ptr<Modio::Detail::OperationQueue> AssociatedQueue = Impl->AssociatedQueue.lock();
						if (AssociatedQueue != nullptr && Impl->OperationQueued)
						{
							AssociatedQueue->Dequeue();
						}
					}
				}

				template<typename OperationType>
				auto WaitForTurnAsync(OperationType&& Operation)
				{
					std::shared_ptr<Modio::Detail::OperationQueue> AssociatedQueue = Impl->AssociatedQueue.lock();
					if (AssociatedQueue == nullptr)
					{
						asio::post(asio::get_associated_executor(Operation), [Op = std::move(Operation)]() mutable {
							Op(Modio::make_error_code(Modio::GenericError::QueueClosed));
						});
						return;
					}
					else
					{
						Impl->OperationQueued = true;
						return AssociatedQueue->Enqueue(std::forward<OperationType>(Operation));
					}
				}
			};

			Ticket GetTicket()
			{
				return Ticket(shared_from_this());
			}

			template<typename OperationType>
			void Enqueue(OperationType&& Operation)
			{
				if (OperationInProgress.exchange(true))
				{
					++NumWaiters;
					// Preserve the associated executor of the queued operation

					QueueImpl.async_wait(
						asio::bind_executor(Modio::Detail::Services::GetGlobalContext().get_executor(),
											[Op = std::move(Operation)](asio::error_code ec) mutable { Op(); }));
				}
				else
				{
					asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Operation));
				}
			}

			void Dequeue()
			{
				OperationInProgress.exchange(false);

				if (NumWaiters > 0)
				{
					--NumWaiters;
					QueueImpl.cancel_one();
				}
			}

			void CancelAll()
			{
				QueueImpl.cancel();
			}
		};
	} // namespace Detail

} // namespace Modio
