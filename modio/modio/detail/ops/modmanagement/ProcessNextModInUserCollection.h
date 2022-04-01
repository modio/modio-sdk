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
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/SaveModCollectionToStorage.h"
#include "modio/detail/ops/mod/SubmitNewModFileOp.h"
#include "modio/detail/ops/modmanagement/InstallOrUpdateMod.h"
#include "modio/detail/ops/modmanagement/UninstallMod.h"
#include "modio/userdata/ModioUserDataService.h"
#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		/// @brief Internal operation. Searches the user's mod collection for the next mod marked as requiring
		/// installation, update, or uninstallation, then performs that operation
		class ProcessNextModInUserCollection
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
					{
						EntryToProcess = nullptr;

						// Note: because we aren't `yield`ing in these loops, we're safe to use range-based for. If we
						// needed to `yield` in the loop, we'd need to store the iterator so we can pick up where we
						// left off when we return from `yield`.

						// Check for pending uninstallations regardless of user
						for (auto ModEntry :
							 Modio::Detail::SDKSessionData::GetSystemModCollection().SortEntriesByRetryPriority())
						{
							if (ModEntry->GetModState() == Modio::ModState::UninstallPending)
							{
								if (ModEntry->ShouldRetry())
								{
									EntryToProcess = ModEntry;
								}
							}
						}

						// If no pending uninstallations, check for this users uploads, installs or updates
						if (!EntryToProcess)
						{
							// if we have a pending upload, process that immediately before bothering with iterating the
							// user subscriptions
							if ((PendingUpload = Modio::Detail::SDKSessionData::GetNextPendingModfileUpload()))
							{
								yield SubmitNewModFileAsync(PendingUpload->first, PendingUpload->second,
															std::move(Self));
								Self.complete(ec);
								return;
							}

							Modio::ModCollection UserModCollection =
								Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();
							for (auto ModEntry : UserModCollection.SortEntriesByRetryPriority())
							{
								Modio::ModState CurrentState = ModEntry->GetModState();
								if (CurrentState == Modio::ModState::InstallationPending ||
									CurrentState == Modio::ModState::UpdatePending)
								{
									if (ModEntry->ShouldRetry())
									{
										EntryToProcess = ModEntry;
									}
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
						// Does this need to be a separate operation or could we provide a parameter to specify
						// we only want to update if it's already installed or something?
						yield Modio::Detail::InstallOrUpdateModAsync(EntryToProcess->GetID(), std::move(Self));
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent {
							EntryToProcess->GetID(),
							EntryToProcess->GetModState() == Modio::ModState::InstallationPending
								? Modio::ModManagementEvent::EventType::Installed
								: Modio::ModManagementEvent::EventType::Updated,
							ec});
						if (ec)
						{
							EntryToProcess->SetLastError(ec);
							Self.complete(ec);
							return;
						}
						else
						{
							yield Modio::Detail::SaveModCollectionToStorageAsync(std::move(Self));

							Self.complete({});
							return;
						}
					}
					else if (EntryToProcess->GetModState() == Modio::ModState::UninstallPending)
					{
						yield Modio::Detail::UninstallModAsync(EntryToProcess->GetID(), std::move(Self));
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent {
							EntryToProcess->GetID(), Modio::ModManagementEvent::EventType::Uninstalled, ec});
						if (ec)
						{
							EntryToProcess->SetLastError(ec);
							Self.complete(ec);
							return;
						}
						else
						{
							yield Modio::Detail::SaveModCollectionToStorageAsync(std::move(Self));

							Self.complete({});
							return;
						}
					}
				}
			}

		private:
			asio::coroutine CoroutineState;
			std::shared_ptr<Modio::ModCollectionEntry> EntryToProcess;
			Modio::Optional<std::pair<Modio::ModID, Modio::CreateModFileParams>> PendingUpload;
		};

		template<typename ProcessNextCallback>
		auto ProcessNextModInUserCollectionAsync(ProcessNextCallback&& OnProcessComplete)
		{
			return asio::async_compose<ProcessNextCallback, void(Modio::ErrorCode)>(
				ProcessNextModInUserCollection(), OnProcessComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>