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
#include "modio/detail/ops/mod/SubmitNewModSourceFileOp.h"
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
						IsTempModSelected = false;

						// Note: because we aren't `yield`ing in these loops, we're safe to use range-based for. If we
						// needed to `yield` in the loop, we'd need to store the iterator so we can pick up where we
						// left off when we return from `yield`.

						// Check for pending uninstallations regardless of user
						{
							auto Lock = Modio::Detail::SDKSessionData::GetReadLock();
							for (auto ModEntry :
								 Modio::Detail::SDKSessionData::GetSystemModCollection().SortEntriesByRetryPriority())
							{
								if (ModEntry->GetModState() == Modio::ModState::UninstallPending)
								{
									if (ModEntry->ShouldRetry())
									{
										// Dont uninstall mod yet if it is still in TempModSet
										if ((Modio::Detail::SDKSessionData::GetTemporaryModSet() != nullptr &&
											 Modio::Detail::SDKSessionData::GetTemporaryModSet()->ContainsModId(
												 ModEntry->GetID())) == false)
										{
											EntryToProcess = ModEntry;
										}
									}
								}
							}
						}

						if (!EntryToProcess)
						{
							for (auto ModEntry :
								 Modio::Detail::SDKSessionData::GetTempModCollection().SortEntriesByRetryPriority())
							{
								if (ModEntry->GetModState() == Modio::ModState::UninstallPending ||
									ModEntry->GetModState() == Modio::ModState::InstallationPending ||
									ModEntry->GetModState() == Modio::ModState::UpdatePending)
								{
									if (ModEntry->ShouldRetry())
									{
										EntryToProcess = ModEntry;
										IsTempModSelected = true;
									}
								}
							}
						}

						// If no pending uninstallations, get the pending priority upload if it exists
						if (!EntryToProcess)
						{
							if ((PendingUpload = Modio::Detail::SDKSessionData::GetPriorityModfileUpload()))
							{
								yield SubmitNewModFileAsync(PendingUpload->first, PendingUpload->second,
															std::move(Self));
								Self.complete(ec);
								return;
							}

							// No priority upload, get priority ID if it exists
							if (Modio::Optional<Modio::ModID> PriorityID =
									Modio::Detail::SDKSessionData::GetPriorityModID())
							{
								Modio::ModCollection UserModCollection =
									Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();
								// If it is set, is it in the user's mod collection?
								if (Modio::Optional<Modio::ModCollectionEntry&> FoundEntry =
										UserModCollection.GetByModID(*PriorityID))
								{
									// If it is, does it need an installation or update?
									Modio::ModState CurrentState = FoundEntry->GetModState();
									if (CurrentState == Modio::ModState::InstallationPending ||
										CurrentState == Modio::ModState::UpdatePending)
									{
										// Has it already been retried too much for this session?
										if (FoundEntry->ShouldRetry())
										{
											// If good to retry, prioritize specified mod download/install
											EntryToProcess = UserModCollection.Entries().at(*PriorityID);
										}
									}
								}
							}
							// If we haven't found an EntryToProcess based on PriorityID, continue to normal uploads and
							// installations
							if (EntryToProcess == nullptr)
							{
								// if we have a pending upload, process that immediately before bothering with iterating
								// the user subscriptions
								if ((PendingUpload = Modio::Detail::SDKSessionData::GetNextPendingModfileUpload()))
								{
									yield SubmitNewModFileAsync(PendingUpload->first, PendingUpload->second,
																std::move(Self));
									Self.complete(ec);
									return;
								}

								// If we don't have a pending mod upload, check for pending source uploads
								if ((PendingSourceUpload =
										 Modio::Detail::SDKSessionData::GetNextPendingSourceFileUpload()))
								{
									yield SubmitNewModSourceFileAsync(PendingSourceUpload->first,
																	  PendingSourceUpload->second, std::move(Self));
									Self.complete(ec);
									return;
								}

								Modio::ModCollection UserModCollection =
									Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();

								// No prioritized mod, sort by normal retry priority
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
													   PendingModState.value() == Modio::ModState::InstallationPending
														   ? Modio::ModManagementEvent::EventType::BeginInstall
														   : Modio::ModManagementEvent::EventType::BeginUpdate,
													   {}});
						// Does this need to be a separate operation or could we provide a parameter to specify
						// we only want to update if it's already installed or something?
						yield Modio::Detail::InstallOrUpdateModAsync(EntryToProcess->GetID(), IsTempModSelected,
																	 std::move(Self));
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
							Modio::ModManagementEvent {EntryToProcess->GetID(),
													   PendingModState.value() == Modio::ModState::InstallationPending
														   ? Modio::ModManagementEvent::EventType::Installed
														   : Modio::ModManagementEvent::EventType::Updated,
													   ec});
						if (ec)
						{
							EntryToProcess->SetLastError(ec);
							Self.complete(ec);
							return;
						}
						else if (!IsTempModSelected)
						{
							yield Modio::Detail::SaveModCollectionToStorageAsync(std::move(Self));
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
															   IsTempModSelected);
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent {
							EntryToProcess->GetID(), Modio::ModManagementEvent::EventType::Uninstalled, ec});

						if (ec)
						{
							EntryToProcess->SetLastError(ec);
							Self.complete(ec);
							return;
						}
						else if (!IsTempModSelected)
						{
							yield Modio::Detail::SaveModCollectionToStorageAsync(std::move(Self));
						}

						Self.complete({});
						return;
					}
				}
			}

		private:
			ModioAsio::coroutine CoroutineState {};
			std::shared_ptr<Modio::ModCollectionEntry> EntryToProcess {};
			Modio::Optional<std::pair<Modio::ModID, Modio::CreateModFileParams>> PendingUpload {};
			Modio::Optional<std::pair<Modio::ModID, Modio::CreateSourceFileParams>> PendingSourceUpload {};
			Modio::Optional<Modio::ModState> PendingModState {};
			bool IsTempModSelected {};
		};

		template<typename ProcessNextCallback>
		auto ProcessNextModInUserCollectionAsync(ProcessNextCallback&& OnProcessComplete)
		{
			return ModioAsio::async_compose<ProcessNextCallback, void(Modio::ErrorCode)>(
				ProcessNextModInUserCollection(), OnProcessComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>