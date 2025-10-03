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
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/SaveModCollectionToStorage.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioTransactionRecordSerialization.h"
#include "modio/userdata/ModioUserDataService.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class PurchaseModOp
		{
		public:
			PurchaseModOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModID,
						  Modio::Optional<uint64_t> PriceInTokens)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  ModId(ModID),
				  PriceInTokens(PriceInTokens)
			{
				RequestParameters = Modio::Detail::PurchaseRequest.SetGameID(GameID)
										.SetModID(ModId)
										.AddQueryParamRaw("idempotent_key", fmt::format("{}", ModId))
										.AddHeaderRaw("X-Modio-Idempotent-Key", fmt::format("{}", ModId));
			}

			/// @brief Secondary constructor for purchases requiring platform-specific values to be embedded in the
			/// purchase request, eg for purchasing via a platform entitlement
			PurchaseModOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModID,
						  Modio::Detail::HttpRequestParams PurchaseViaEntitlementRequest)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  ModId(ModID),
				  RequestParameters(PurchaseViaEntitlementRequest)
			{
				RequestParameters = RequestParameters.SetGameID(GameID)
										.SetModID(ModId)
										.AddQueryParamRaw("idempotent_key", fmt::format("{}", ModId))
										.AddHeaderRaw("X-Modio-Idempotent-Key", fmt::format("{}", ModId));
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if (PriceInTokens.has_value())
				{
					RequestParameters.AddQueryParamRaw("display_amount", fmt::format("{}", PriceInTokens.value()));
				}

				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(ResponseBodyBuffer, RequestParameters,
																		   Modio::Detail::CachedResponse::Disallow,
																		   std::move(Self));

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
					}
					if (Modio::ErrorCodeMatches(ec, Modio::ApiError::MonetizationIncorrectDisplayPrice))
					{
						Self.complete(Modio::make_error_code(Modio::MonetizationError::IncorrectDisplayPrice), {});
						return;
					}
					if (Modio::ErrorCodeMatches(ec, Modio::ApiError::MonetizationAccountLacksEntitlement))
					{
						Self.complete(Modio::make_error_code(Modio::MonetizationError::AccountLacksEntitlement), {});
					}
					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						Record = TryMarshalResponse<Modio::TransactionRecord>(ResponseBodyBuffer);
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

							auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();

							Services::GetGlobalService<CacheService>().AddToCache(Record.value().Mod);
							Modio::Detail::SDKSessionData::GetSystemModCollection().AddOrUpdateMod(
								Record.value().Mod,
								Modio::ToModioString(
									Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
										.MakeModPath(Record.value().Mod.ModId)
										.u8string()));

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
			Modio::GameID GameID {};
			Modio::ApiKey ApiKey {};
			Modio::ModID ModId {};
			Modio::Optional<uint64_t> PriceInTokens {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			asio::coroutine CoroutineState {};
			Modio::Optional<Modio::TransactionRecord> Record {};
			Modio::Detail::HttpRequestParams RequestParameters;
		};
#include <asio/unyield.hpp>

		template<typename PurchaseCompleteCallback>
		void PurchaseModAsync(Modio::ModID ModID, Modio::Optional<uint64_t> ExpectedVirtualCurrencyPrice,
							  PurchaseCompleteCallback&& OnPurchaseComplete)
		{
			return asio::async_compose<PurchaseCompleteCallback,
									   void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>(
				Modio::Detail::PurchaseModOp(Modio::Detail::SDKSessionData::CurrentGameID(),
											 Modio::Detail::SDKSessionData::CurrentAPIKey(), ModID,
											 ExpectedVirtualCurrencyPrice),
				OnPurchaseComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
