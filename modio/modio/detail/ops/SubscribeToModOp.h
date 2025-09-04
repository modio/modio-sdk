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
#include "modio/core/ModioErrorCode.h"
#include "modio/detail/serialization/ModioModCollectionEntrySerialization.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/SaveModCollectionToStorage.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/file/ModioFileService.h"
#include "modio/userdata/ModioUserDataService.h"
#include <asio/coroutine.hpp>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class SubscribeToModOp
		{
		public:
			SubscribeToModOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModID, bool IncludeDependencies)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  ModId(ModID),
				  IncludeDependencies(IncludeDependencies)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					Modio::Detail::SDKSessionData::RemoveDeferredUnsubscription(ModId);
					Modio::Detail::SDKSessionData::DecrementModManagementEventQueued();

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::SubscribeToModRequest.SetGameID(GameID).SetModID(ModId).AddQueryParamRaw(
							Modio::Detail::Constants::QueryParamStrings::IncludeDependecies, (IncludeDependencies ? "true" : "false")),
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
													"Already subscribed to mod {}", ModId);
						Self.complete({});
						return;
					}
					else if (ec)
					{
						Self.complete(ec);
						return;
					}

					{
						Modio::Optional<Modio::ModInfo> ProfileData =
							TryMarshalResponse<Modio::ModInfo>(ResponseBodyBuffer);
						if (!ProfileData.has_value())
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}

						Modio::Detail::SDKSessionData::InvalidateSubscriptionCache();

						Services::GetGlobalService<CacheService>().AddToCache(ProfileData.value());
						Modio::Detail::SDKSessionData::GetSystemModCollection().AddOrUpdateMod(
							ProfileData.value(),
							Modio::ToModioString(
								Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
									.MakeModPath(ProfileData->ModId).u8string()));
						// Returns true if this is a new subscription for the user
						if (Modio::Detail::SDKSessionData::GetUserSubscriptions().AddMod(ProfileData.value()))
						{
							// Increment the reference count
							std::uint8_t ReferenceCount =
								Modio::Detail::SDKSessionData::GetSystemModCollection()
									.Entries()
									.at(ProfileData->ModId)
									->AddLocalUserSubscription(Modio::Detail::SDKSessionData::GetAuthenticatedUser());

							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::ModManagement,
														"Incremented reference count for mod {} to {}", ModId,
														ReferenceCount);

							// Currently not checking the return values from these calls, because if they fail we'll
							// still be installing the mod yield
							// Modio::Detail::async_CommitSystemModCollection(std::move(Self)); yield
							// Modio::Detail::async_CommitCurrentUserSubscriptions(std::move(Self));
							Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
														"Successfully subscribed to mod {}", ModId);
						}
						else
						{
							// Inform the caller the user is already subscribed
							Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
														"Already subscribed to mod {}", ModId);
						}
					}
					yield Modio::Detail::SaveModCollectionToStorageAsync(std::move(Self));
					// probably should be in the true branch of the if statement above
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
			Modio::ApiKey ApiKey {};
			Modio::ModID ModId {};
			bool IncludeDependencies = false;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			asio::coroutine CoroutineState {};
		};

		template<typename SubscribeCompleteCallback>
		void SubscribeToModAsync(Modio::ModID ModToSubscribeTo, bool IncludeDependencies,
								 SubscribeCompleteCallback&& OnSubscribeComplete)
		{
			return asio::async_compose<SubscribeCompleteCallback, void(Modio::ErrorCode)>(
				Modio::Detail::SubscribeToModOp(Modio::Detail::SDKSessionData::CurrentGameID(),
												Modio::Detail::SDKSessionData::CurrentAPIKey(),
												ModToSubscribeTo, IncludeDependencies),
				OnSubscribeComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
