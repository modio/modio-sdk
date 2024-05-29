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
#include "modio/core/ModioServices.h"
#include "modio/core/entities/ModioTransactionRecord.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/ops/SaveModCollectionToStorage.h"
#include "modio/userdata/ModioUserDataService.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class PurchaseModOp
		{
		public:
			PurchaseModOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModID, uint64_t PriceInTokens)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  ModId(ModID), 
				  PriceInTokens(PriceInTokens)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::PurchaseRequest.SetGameID(GameID).SetModID(ModId).AddQueryParamRaw("display_amount", fmt::format("{}", PriceInTokens))
							.AddQueryParamRaw("idempotent_key", fmt::format("{}", ModId))
							.AddHeaderRaw("X-Modio-Idempotent-Key", fmt::format("{}", ModId)),
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
					}
					if (Modio::ErrorCodeMatches(ec, Modio::MonetizationError::IncorrectDisplayPrice))
					{
						Self.complete(Modio::make_error_code(Modio::MonetizationError::IncorrectDisplayPrice), {});
						return;
					}
					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						Record =
							TryMarshalResponse<Modio::TransactionRecord>(ResponseBodyBuffer);
						if (!Record.has_value())
						{
							Modio::Detail::SDKSessionData::InvalidatePurchaseCache();
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
							return;
						}
						(*Record).AssociatedModID = ModId;

						// Now that we have the mod, we need to update the local subscription cache with it
						{
							Modio::Detail::SDKSessionData::InvalidateSubscriptionCache();

							Services::GetGlobalService<CacheService>().AddToCache(Record.value().Mod);
							Modio::Detail::SDKSessionData::GetSystemModCollection().AddOrUpdateMod(
								Record.value().Mod,
								Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
									.MakeModPath(Record.value().Mod.ModId)
									.u8string());

							if (Modio::Detail::SDKSessionData::GetModPurchases().count(Record.value().Mod.ModId) > 0)
							{
								Modio::Detail::SDKSessionData::GetModPurchases()[Record.value().Mod.ModId] =
									Record.value().Mod;
							}
							else
							{
								Modio::Detail::SDKSessionData::GetModPurchases().emplace(Record.value().Mod.ModId,
																						 Record.value().Mod);
							}

							// Returns true if this is a new subscription for the user
							if (Modio::Detail::SDKSessionData::GetUserSubscriptions().AddMod(Record.value().Mod))
							{
								// Increment the reference count
								std::uint8_t ReferenceCount =
									Modio::Detail::SDKSessionData::GetSystemModCollection()
										.Entries()
										.at(Record.value().Mod.ModId)
										->AddLocalUserSubscription(
											Modio::Detail::SDKSessionData::GetAuthenticatedUser());

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

						Self.complete({}, Record);
					}
				}
			}

		private:
			Modio::GameID GameID;
			Modio::ApiKey ApiKey;
			Modio::ModID ModId;
			uint64_t PriceInTokens = 0;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			asio::coroutine CoroutineState;
			Modio::Optional<Modio::TransactionRecord> Record;
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
