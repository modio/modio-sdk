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
#include "modio/detail/ops/monetization/GetAvailableUserEntitlementsGoogle.h"
#include "modio/detail/ops/monetization/GetAvailableUserEntitlementsMeta.h"
#include "modio/detail/ops/monetization/GetAvailableUserEntitlementsPSN.h"
#include "modio/detail/ops/monetization/GetAvailableUserEntitlementsSteam.h"
#include "modio/detail/ops/monetization/GetAvailableUserEntitlementsXboxLive.h"
#include "modio/detail/ops/monetization/GetUserDelegationTokenOp.h"
#include "modio/detail/ops/monetization/GetUserWalletBalanceOp.h"
#include "modio/detail/ops/monetization/PurchaseModOp.h"
#include "modio/detail/ops/monetization/PurchaseModWithEntitlementGoogle.h"
#include "modio/detail/ops/monetization/PurchaseModWithEntitlementMeta.h"
#include "modio/detail/ops/monetization/PurchaseModWithEntitlementPSN.h"
#include "modio/detail/ops/monetization/PurchaseModWithEntitlementSteam.h"
#include "modio/detail/ops/monetization/PurchaseModWithEntitlementXboxLive.h"
#include "modio/detail/ops/monetization/RefreshUserEntitlementsGoogle.h"
#include "modio/detail/ops/monetization/RefreshUserEntitlementsMeta.h"
#include "modio/detail/ops/monetization/RefreshUserEntitlementsPSN.h"
#include "modio/detail/ops/monetization/RefreshUserEntitlementsSteam.h"
#include "modio/detail/ops/monetization/RefreshUserEntitlementsXboxLive.h"
#include "modio/detail/serialization/ModioEntitlementConsumptionStatusListSerialization.h"
#include "modio/detail/serialization/ModioEntitlementConsumptionStatusSerialization.h"
#include "modio/detail/serialization/ModioResponseErrorSerialization.h"
#include "modio/detail/serialization/ModioTransactionRecordSerialization.h"
#include "modio/detail/serialization/ModioUserDelegationTokenSerialization.h"
#include "modio/impl/SDKPreconditionChecks.h"

// Implementation header - do not include directly

namespace Modio
{
	void PurchaseModAsync(Modio::ModID ModID, Modio::Optional<uint64_t> ExpectedVirtualCurrencyPrice,
						  std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModID, ExpectedVirtualCurrencyPrice,
													Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireValidModID(ModID, Callback) && Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) && Modio::Detail::RequireNotRateLimited(Callback))
			{
				Modio::Detail::PurchaseModAsync(ModID, ExpectedVirtualCurrencyPrice, Callback);
			}
		});
	}

	void PurchaseModWithEntitlementAsync(
		Modio::ModID ModID, Modio::EntitlementParams Params,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModID, Params, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireValidModID(ModID, Callback) == false &&
				Modio::Detail::RequireSDKIsInitialized(Callback) == false &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) == false &&
				Modio::Detail::RequireNotRateLimited(Callback) == false)
			{
				return;
			}

			switch (Detail::SDKSessionData::GetPortal())
			{
				case Modio::Portal::Steam:
					Modio::Detail::PurchaseModWithEntitlementSteamAsync(ModID, Params, Callback);
					break;

				case Modio::Portal::XboxLive:
					if (Modio::Detail::RequireValidXBoxRefreshEntitlementsExtendedParameters(Params, Callback) == false)
					{
						return;
					}

					Modio::Detail::PurchaseModWithEntitlementXboxLiveAsync(ModID, Params, Callback);
					break;

				case Modio::Portal::PSN:
					if (Modio::Detail::RequireValidPSNRefreshEntitlementsExtendedParameters(Params, Callback) == false)
					{
						return;
					}

					Modio::Detail::PurchaseModWithEntitlementPSNAsync(ModID, Params, Callback);
					break;

				case Modio::Portal::Google:
					if (Modio::Detail::RequireValidGoogleRefreshEntitlementsExtendedParameters(Params, Callback) ==
						false)
					{
						return;
					}

					Modio::Detail::PurchaseModWithEntitlementGoogleAsync(ModID, Params, Callback);
					break;

				case Modio::Portal::Meta:
					if (Modio::Detail::RequireValidMetaRefreshEntitlementsExtendedParameters(Params, Callback) == false)
					{
						return;
					}

					Modio::Detail::PurchaseModWithEntitlementMetaAsync(ModID, Params, Callback);
					break;

				case Modio::Portal::None:
				case Modio::Portal::Apple:
				case Modio::Portal::EpicGamesStore:
				case Modio::Portal::GOG:
				case Modio::Portal::Itchio:
				case Modio::Portal::Nintendo:
				default:
					Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::ModManagement,
												"Called RefreshEntitlements with an unsupported Portal of {}",
												Detail::SDKSessionData::GetPortal());

					ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
							   [CompletionHandler = std::forward<
									std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>>(
									Callback)]() mutable {
								   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter), {});
							   });
					return;
			}
		});
	}

	void RefreshUserEntitlementsAsync(
		Modio::EntitlementParams Params,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Params, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) == false &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) == false &&
				Modio::Detail::RequireNotRateLimited(Callback) == false)
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

				case Modio::Portal::Google:
					if (Modio::Detail::RequireValidGoogleRefreshEntitlementsExtendedParameters(Params, Callback) ==
						false)
					{
						return;
					}

					Modio::Detail::RefreshUserEntitlementsGoogleAsync(Params, Callback);
					break;

				case Modio::Portal::Meta:
					if (Modio::Detail::RequireValidMetaRefreshEntitlementsExtendedParameters(Params, Callback) == false)
					{
						return;
					}

					Modio::Detail::RefreshUserEntitlementsMetaAsync(Params, Callback);
					break;

				case Modio::Portal::None:
				case Modio::Portal::Apple:
				case Modio::Portal::EpicGamesStore:
				case Modio::Portal::GOG:
				case Modio::Portal::Itchio:
				case Modio::Portal::Nintendo:
				default:
					Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::ModManagement,
												"Called RefreshEntitlements with an unsupported Portal of {}",
												Detail::SDKSessionData::GetPortal());

					ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
							   [CompletionHandler = std::forward<std::function<void(
									Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>>(
									Callback)]() mutable {
								   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter), {});
							   });
					return;
			}
		});
	}

	void GetAvailableUserEntitlementsAsync(
		Modio::EntitlementParams Params,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Params, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) == false &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) == false &&
				Modio::Detail::RequireNotRateLimited(Callback) == false)
			{
				return;
			}

			switch (Detail::SDKSessionData::GetPortal())
			{
				case Modio::Portal::Steam:
					Modio::Detail::GetAvailableUserEntitlementsSteamAsync(Params, Callback);
					break;

				case Modio::Portal::XboxLive:
					if (Modio::Detail::RequireValidXBoxRefreshEntitlementsExtendedParameters(Params, Callback) == false)
					{
						return;
					}

					Modio::Detail::GetAvailableUserEntitlementsXBoxLiveAsync(Params, Callback);
					break;

				case Modio::Portal::PSN:
					if (Modio::Detail::RequireValidPSNRefreshEntitlementsExtendedParameters(Params, Callback) == false)
					{
						return;
					}

					Modio::Detail::GetAvailableUserEntitlementsPSNAsync(Params, Callback);
					break;

				case Modio::Portal::Google:
					if (Modio::Detail::RequireValidGoogleRefreshEntitlementsExtendedParameters(Params, Callback) ==
						false)
					{
						return;
					}

					Modio::Detail::GetAvailableUserEntitlementsGoogleAsync(Params, Callback);
					break;

				case Modio::Portal::Meta:
					if (Modio::Detail::RequireValidMetaRefreshEntitlementsExtendedParameters(Params, Callback) == false)
					{
						return;
					}

					Modio::Detail::GetAvailableUserEntitlementsMetaAsync(Params, Callback);
					break;

				case Modio::Portal::None:
				case Modio::Portal::Apple:
				case Modio::Portal::EpicGamesStore:
				case Modio::Portal::GOG:
				case Modio::Portal::Itchio:
				case Modio::Portal::Nintendo:
				default:
					Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::ModManagement,
												"Called GetAvailableEntitlements with an unsupported Portal of {}",
												Detail::SDKSessionData::GetPortal());

					ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
							   [CompletionHandler = std::forward<
									std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>>(
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
				Modio::Detail::GetUserWalletBalanceAsync(Callback);
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
				Modio::Detail::GetUserDelegationTokenAsync(Callback);
			}
		});
	}
}; // namespace Modio