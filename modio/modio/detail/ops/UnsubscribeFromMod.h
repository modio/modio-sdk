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
#include "modio/detail/CoreOps.h"
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
			UnsubscribeFromModOp(Modio::GameID GameID, Modio::ModID ModId) : GameID(GameID), ModId(ModId) {};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					
					Modio::Detail::SDKSessionData::CancelModDownloadOrUpdate(ModId);

					Modio::Detail::SDKSessionData::GetUserSubscriptions().RemoveMod(ModId);

					{
						Modio::Optional<Modio::ModCollectionEntry&> ModEntry =
							Modio::Detail::SDKSessionData::GetSystemModCollection().GetByModID(ModId);
						if (ModEntry.has_value())
						{
							ModEntry->RemoveLocalUserSubscription(
								Modio::Detail::SDKSessionData::GetAuthenticatedUser());
						}
					}

					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, Modio::Detail::UnsubscribeFromModRequest.SetGameID(GameID).SetModID(ModId),
						Modio::Detail::CachedResponse::Allow, std::move(Self));
					
					if (ec == Modio::ErrorConditionTypes::ApiErrorRefSuccess)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
													"User was not subscribed to mod {}", ModId);
					} 
					else if (ec)
					{
						if (ec != Modio::GenericError::OperationCanceled)
						{
							Modio::Detail::SDKSessionData::AddToDeferredUnsubscriptions(ModId);
						}
						Self.complete(ec);
						return;
					}

					
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
			Modio::GameID GameID;
			Modio::ModID ModId;
			asio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
		};

		template<typename UnsubscribeCompleteCallback>
		void UnsubscribeFromModAsync(Modio::ModID ModToUnsubscribeFrom,
									 UnsubscribeCompleteCallback&& OnUnsubscribeComplete)
		{
			return asio::async_compose<UnsubscribeCompleteCallback, void(Modio::ErrorCode)>(
				Modio::Detail::UnsubscribeFromModOp(Modio::Detail::SDKSessionData::CurrentGameID(), ModToUnsubscribeFrom),
				OnUnsubscribeComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio