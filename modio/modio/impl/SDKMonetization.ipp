/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/ModioSDK.h"
#else
	#pragma once
#endif

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/monetization/FetchUserPurchasesOp.h"
#include "modio/detail/ops/monetization/GetUserDelegationTokenOp.h"
#include "modio/detail/ops/monetization/GetUserWalletBalanceOp.h"
#include "modio/detail/ops/monetization/PurchaseModOp.h"
#include "modio/detail/serialization/ModioResponseErrorSerialization.h"
#include "modio/detail/serialization/ModioTransactionRecordSerialization.h"
#include "modio/detail/serialization/ModioUserDelegationTokenSerialization.h"
#include "modio/detail/serialization/ModioEntitlementConsumptionStatusSerialization.h"
#include "modio/detail/serialization/ModioEntitlementConsumptionStatusListSerialization.h"
#include "modio/detail/ops/monetization/GetUserWalletBalanceOp.h"
#include "modio/detail/ops/monetization/PurchaseModOp.h"
#include "modio/detail/ops/monetization/RefreshUserEntitlementsSteam.h"
#include "modio/detail/ops/monetization/RefreshUserEntitlementsXboxLive.h"
#include "modio/detail/ops/monetization/RefreshUserEntitlementsPSN.h"
#include "modio/impl/SDKPreconditionChecks.h"

// Implementation header - do not include directly

namespace Modio
{
	void PurchaseModAsync(Modio::ModID ModID, uint64_t ExpectedPrice,
						  std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModID, ExpectedPrice, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireValidModID(ModID, Callback) && Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) && Modio::Detail::RequireNotRateLimited(Callback))
			{
				asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>,
									void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>(
					Modio::Detail::PurchaseModOp(Modio::Detail::SDKSessionData::CurrentGameID(),
												 Modio::Detail::SDKSessionData::CurrentAPIKey(), ModID, ExpectedPrice),
					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

	void RefreshUserEntitlementsAsync(
		Modio::EntitlementParams Params,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Params, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) == false &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) == false && Modio::Detail::RequireNotRateLimited(Callback) == false)
			{
				return;
			}

				switch (Detail::SDKSessionData::GetPortal())
				{
					case Modio::Portal::Steam:
					Modio::Detail::RefreshUserEntitlementsSteamAsync(Params, Callback);
					break;

					case Modio::Portal::XboxLive:
					if (Modio::Detail::RequireValidXBoxRefreshEntitlementsExtendedParameters(Params, Callback) == false)
					{
						return;
					}

					Modio::Detail::RefreshUserEntitlementsXboxLiveAsync(Params, Callback);
					break;

					case Modio::Portal::PSN:
					if (Modio::Detail::RequireValidPSNRefreshEntitlementsExtendedParameters(Params, Callback) == false)
					{
						return;
					}

					Modio::Detail::RefreshUserEntitlementsPSNAsync(Params, Callback);
					break;

					default:
					Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::ModManagement,
												"Called RefreshEntitlements with an unsupported Portal of {}",
												Detail::SDKSessionData::GetPortal());

					asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
							   [CompletionHandler = std::forward<std::function<void(
									Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>>(
									Callback)]() mutable {
								   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter), {});
							   });
					return;
			}
		});
	}

	void GetUserWalletBalanceAsync(std::function<void(Modio::ErrorCode, Modio::Optional<uint64_t>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) && Modio::Detail::RequireNotRateLimited(Callback))
			{
				asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<uint64_t>)>,
									void(Modio::ErrorCode, Modio::Optional<uint64_t>)>(
					Modio::Detail::GetUserWalletBalanceOp(Modio::Detail::SDKSessionData::CurrentGameID(),
														  Modio::Detail::SDKSessionData::CurrentAPIKey()),
					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

	void FetchUserPurchasesAsync(std::function<void(Modio::ErrorCode)> OnFetchDone)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([OnFetchDone = std::move(OnFetchDone)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(OnFetchDone) &&
				Modio::Detail::RequireUserIsAuthenticated(OnFetchDone) &&
				Modio::Detail::RequireNotRateLimited(OnFetchDone))
			{
				Modio::Detail::FetchUserPurchasesAsync(OnFetchDone);
			}
		});
	}

	std::map<Modio::ModID, Modio::ModInfo> QueryUserPurchasedMods()
	{
		auto Lock = Modio::Detail::SDKSessionData::GetReadLock();
		if (Modio::Detail::SDKSessionData::IsInitialized())
		{
			auto UserModPuchases = Modio::Detail::SDKSessionData::GetModPurchases();

			return UserModPuchases;
		}
		else
		{
			return {};
		}
	}

	void GetUserDelegationTokenAsync(std::function<void(Modio::ErrorCode, std::string)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) && Modio::Detail::RequireNotRateLimited(Callback))
			{
				asio::async_compose<std::function<void(Modio::ErrorCode, std::string)>,
									void(Modio::ErrorCode, std::string)>(
					Modio::Detail::GetUserDelegationTokenOp(Modio::Detail::SDKSessionData::CurrentGameID(),
															Modio::Detail::SDKSessionData::CurrentAPIKey()),
					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}
}; // namespace Modio