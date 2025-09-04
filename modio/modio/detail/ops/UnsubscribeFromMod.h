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
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/userdata/ModioUserDataService.h"
#include <asio/coroutine.hpp>
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class UnsubscribeFromModOp
		{
		public:
			UnsubscribeFromModOp(Modio::GameID GameID, Modio::ModID ModId)
				: GameID(GameID),
				  ModId(ModId) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					Modio::Detail::SDKSessionData::CancelModDownloadOrUpdate(ModId);

					Modio::Detail::SDKSessionData::GetUserSubscriptions().RemoveMod(ModId);
					Modio::Detail::SDKSessionData::DecrementModManagementEventQueued();

					{
						Modio::Optional<Modio::ModCollectionEntry&> ModEntry =
							Modio::Detail::SDKSessionData::GetSystemModCollection().GetByModID(ModId);
						if (ModEntry.has_value())
						{
							std::uint8_t ReferenceCount = ModEntry->RemoveLocalUserSubscription(
								Modio::Detail::SDKSessionData::GetAuthenticatedUser());

							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::ModManagement,
														"Decremented reference count for mod {} to {}", ModId,
														ReferenceCount);
						}
					}

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::UnsubscribeFromModRequest.SetGameID(GameID).SetModID(ModId),
						Modio::Detail::CachedResponse::Allow, std::move(Self));

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
						Self.complete(ec);
						return;
					}

					if (ec == Modio::ErrorConditionTypes::ApiErrorRefSuccess)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
													"User was not subscribed to mod {}", ModId);
					}
					else if (ec)
					{
						if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::EntityNotFoundError))
						{
							Modio::Detail::Logger().Log(
								Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
								"Attempted to unsubscribe from mod {} but that mod does not exist. Returning success",
								ModId);
							Self.complete({});
							return;
						}
						if (ec != Modio::GenericError::OperationCanceled)
						{
							Modio::Detail::SDKSessionData::AddToDeferredUnsubscriptions(ModId);
							Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
														"Attempted to unsubscribe from mod {}, received error code {}. "
														"Deferring unsubscription.",
														ModId, ec.value());
							// Treating as success when unsubscription is deferred to facilitate UE4 UI functionality
							Self.complete({});
							return;
						}
						Self.complete(ec);
						return;
					}

					Modio::Detail::SDKSessionData::InvalidateSubscriptionCache();

					yield Modio::Detail::SaveModCollectionToStorageAsync(std::move(Self));
					yield Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>()
						.SaveUserDataToStorageAsync(std::move(Self));
					// No need to check return from SaveUserData, because the subscription is successful regardless of
					// flushing the user storage state

					Self.complete({});
					return;
				}
			}

		private:
			Modio::GameID GameID {};
			Modio::ModID ModId {};
			asio::coroutine CoroutineState {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
		};

		template<typename UnsubscribeCompleteCallback>
		void UnsubscribeFromModAsync(Modio::ModID ModToUnsubscribeFrom,
									 UnsubscribeCompleteCallback&& OnUnsubscribeComplete)
		{
			return asio::async_compose<UnsubscribeCompleteCallback, void(Modio::ErrorCode)>(
				Modio::Detail::UnsubscribeFromModOp(Modio::Detail::SDKSessionData::CurrentGameID(),
													ModToUnsubscribeFrom),
				OnUnsubscribeComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio