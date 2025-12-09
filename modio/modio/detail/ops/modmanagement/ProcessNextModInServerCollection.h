/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioLogger.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/modmanagement/InstallOrUpdateMod.h"
#include "modio/detail/ops/modmanagement/UninstallMod.h"
#include "modio/userdata/ModioUserDataService.h"
#include "modio/detail/ModioSDKMultiplayerLibrary.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		/// @brief Internal process, searches through mod collections for the next mod pending an operation
		/// then performs that operation, specifically for dedicated servers
		class ProcessNextModInServerCollection
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}
				reenter(CoroutineState)
				{
					EntryToProcess = nullptr;

					// Note: because we aren't `yield`ing in these loops, we're safe to use range-based for. If we
					// needed to `yield` in the loop, we'd need to store the iterator so we can pick up where we
					// left off when we return from `yield`.
					if (Modio::ModioServer::IsValid())
					{
						{
							auto Lock = Modio::Detail::SDKSessionData::GetReadLock();
							for (auto ModEntry : Modio::Detail::SDKSessionData::GetSystemModCollection()
													 .SortEntriesByRetryPriority())
							{
								if (ModEntry->GetModState() == Modio::ModState::InstallationPending ||
									ModEntry->GetModState() == Modio::ModState::UninstallPending ||
									ModEntry->GetModState() == Modio::ModState::UpdatePending)
								{
									if (ModEntry->ShouldRetry())
									{
										EntryToProcess = ModEntry;
										break;
									}
								}
							}
						}

						if (EntryToProcess == nullptr)
						{
							Self.complete({});
							return;
						}

						if (EntryToProcess->GetModState() == Modio::ModState::InstallationPending ||
							EntryToProcess->GetModState() == Modio::ModState::UpdatePending)
						{
							PendingModState = EntryToProcess->GetModState();

							Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
								Modio::ModManagementEvent {EntryToProcess->GetID(),
														   PendingModState.value() ==
																   Modio::ModState::InstallationPending
															   ? Modio::ModManagementEvent::EventType::BeginInstall
															   : Modio::ModManagementEvent::EventType::BeginUpdate,
														   {}});
							// Does this need to be a separate operation or could we provide a parameter to specify
							// we only want to update if it's already installed or something?
							yield Modio::Detail::InstallOrUpdateModAsync(EntryToProcess->GetID(), false,
																		 std::move(Self));
							Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
								Modio::ModManagementEvent {EntryToProcess->GetID(),
														   PendingModState.value() ==
																   Modio::ModState::InstallationPending
															   ? Modio::ModManagementEvent::EventType::Installed
															   : Modio::ModManagementEvent::EventType::Updated,
														   ec});
							if (ec)
							{
								EntryToProcess->SetLastError(ec);
								Self.complete(ec);
								return;
							}

							Self.complete({});
							return;
						}
						else if (EntryToProcess->GetModState() == Modio::ModState::UninstallPending)
						{
							Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
								Modio::ModManagementEvent {EntryToProcess->GetID(),
														   Modio::ModManagementEvent::EventType::BeginUninstall,
														   {}});

							yield Modio::Detail::UninstallModAsync(EntryToProcess->GetID(), std::move(Self), false,
																   false);
							Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
								Modio::ModManagementEvent {EntryToProcess->GetID(),
														   Modio::ModManagementEvent::EventType::Uninstalled, ec});

							if (ec)
							{
								EntryToProcess->SetLastError(ec);
								Self.complete(ec);
								return;
							}

							Self.complete({});
							return;
						}
					}
				}
			}

		private:
			ModioAsio::coroutine CoroutineState;
			std::shared_ptr<Modio::ModCollectionEntry> EntryToProcess;
			Modio::Optional<Modio::ModState> PendingModState;
		};

		template<typename ProcessNextCallback>
		auto ProcessNextModInServerCollectionAsync(ProcessNextCallback&& OnProcessCollection)
		{
			return ModioAsio::async_compose<ProcessNextCallback, void(Modio::ErrorCode)>(
				ProcessNextModInServerCollection(), OnProcessCollection,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
