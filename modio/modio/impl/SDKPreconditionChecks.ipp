/*
 *  Copyright (C) 2021-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */


#include "modio/core/ModioEditModParams.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioMetricsService.h"
#include "modio/core/ModioModDependency.h"
#include "modio/core/ModioReportParams.h"
#include "modio/core/ModioServices.h"
#include "modio/core/ModioServerInitializeOptions.h"
#include "modio/core/entities/ModioModCollection.h"
#include "modio/core/entities/ModioTransactionRecord.h"
#include "modio/core/entities/ModioGameInfo.h"
#include "modio/core/entities/ModioGameInfoList.h"
#include "modio/core/entities/ModioUserList.h"
#include "modio/core/entities/ModioUserRatingList.h"
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/core/entities/ModioTerms.h"
#include "modio/core/entities/ModioEntitlement.h"
#include "modio/core/entities/ModioEntitlementConsumptionStatusList.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ModioConstants.h"
#include "modio/file/ModioFileService.h"
#include "modio/impl/SDKPreconditionChecks.h"

namespace Modio
{
	namespace Detail
	{
		template<typename... OtherArgs>
		bool RequireSDKIsInitialized(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::IsInitialized())
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::SDKNotInitialized),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::ModCollectionInfoList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfoList>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::ModCollectionInfo>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfo>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::ModInfo>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::ModID>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModID>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::Terms>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::Terms>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<std::string>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)>&);
		template bool RequireSDKIsInitialized<std::string>(std::function<void(Modio::ErrorCode, std::string)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<std::uint64_t>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<std::uint64_t>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::ModInfoList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::ModTagOptions>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModTagOptions>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::ModDependencyList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDependencyList>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::TransactionRecord>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::UserList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::GameInfo>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfo>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::UserSubscriptionListChangeType>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserSubscriptionListChangeType>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::GameInfoList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfoList>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::UserRatingList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserRatingList>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::EntitlementConsumptionStatusList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>&);
		template bool RequireSDKIsInitialized<std::map<Modio::ModID, Modio::UserSubscriptionListChangeType>>(
			std::function<void(Modio::ErrorCode, std::map<Modio::ModID, Modio::UserSubscriptionListChangeType>)>&);
		template bool RequireSDKIsInitialized<std::set<Modio::ModID>>(
			std::function<void(Modio::ErrorCode, std::set<Modio::ModID>)>&);
		template bool RequireSDKIsInitialized<Modio::Optional<Modio::EntitlementList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>&);
		template bool RequireSDKIsInitialized<>(std::function<void(Modio::ErrorCode)>&);

		template<typename... OtherArgs>
		bool RequireUserIsAuthenticated(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::GetAuthenticatedUser().has_value())
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::UserDataError::InvalidUser),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireUserIsAuthenticated<>(std::function<void(Modio::ErrorCode)>& Handler);
		template bool RequireUserIsAuthenticated<std::string>(
			std::function<void(Modio::ErrorCode, std::string)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<std::uint64_t>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<std::uint64_t>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::EntitlementConsumptionStatusList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::EntitlementList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::GameInfoList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfoList>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::ModID>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModID>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::ModInfo>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::ModInfoList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::ModCollectionInfo>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfo>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::ModCollectionInfoList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfoList>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::TransactionRecord>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::UserList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<Modio::UserRatingList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserRatingList>)>& Handler);
		template bool RequireUserIsAuthenticated<Modio::Optional<std::string>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)>& Handler);
		template bool RequireUserIsAuthenticated<std::map<Modio::ModID, Modio::UserSubscriptionListChangeType>>(
			std::function<void(Modio::ErrorCode, std::map<Modio::ModID, Modio::UserSubscriptionListChangeType>)>&
				Handler);

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsInitialized(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().IsInitialized())
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::MetricsError::SessionNotInitialized),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}

		template bool RequireMetricsSessionIsInitialized<>(std::function<void(Modio::ErrorCode)>&);

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsActive(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().GetSessionIsActive())
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::MetricsError::SessionIsNotActive),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireMetricsSessionIsActive<>(std::function<void(Modio::ErrorCode)>&);

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsNOTActive(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().GetSessionIsActive())
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::MetricsError::SessionIsActive),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireMetricsSessionIsNOTActive<>(std::function<void(Modio::ErrorCode)>&);

		template<typename... OtherArgs>
		bool RequireValidMetricSessionParams(const Modio::MetricsSessionParams& Params,
											 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			bool bInvalidParameter = false;
			if (Params.ModIds.empty())
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Mod Ids are empty");
				bInvalidParameter = true;
			}
			else
			{
				for (Modio::ModID mod_id : Params.ModIds)
				{
					if (!mod_id.IsValid())
					{
						Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Mod Id is invalid");
						bInvalidParameter = true;
						break;
					}
				}
			}

			if (bInvalidParameter)
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
			else
			{
				return true;
			}
		}
		template bool RequireValidMetricSessionParams<>(const Modio::MetricsSessionParams&,
														std::function<void(Modio::ErrorCode)>&);

		template<typename... OtherArgs>
		bool RequireNotRateLimited(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::SDKSessionData::IsRateLimited())
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::HttpError::RateLimited),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireNotRateLimited<>(std::function<void(Modio::ErrorCode)>&);
		template bool RequireNotRateLimited<std::string>(std::function<void(Modio::ErrorCode, std::string)>&);
		template bool RequireNotRateLimited<Modio::Optional<std::uint64_t>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<std::uint64_t>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::ModID>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModID>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::UserList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::UserRatingList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserRatingList>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::GameInfo>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfo>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::GameInfoList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfoList>)>&);
		template bool RequireNotRateLimited<Modio::Optional<std::string>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::ModInfo>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::ModInfoList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::ModTagOptions>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModTagOptions>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::ModDependencyList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDependencyList>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::ModCollectionInfoList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfoList>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::TransactionRecord>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::EntitlementList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>&);
		template bool RequireNotRateLimited<Modio::Optional<Modio::EntitlementConsumptionStatusList>>(
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>&);
		template bool RequireNotRateLimited<std::map<Modio::ModID, Modio::UserSubscriptionListChangeType>>(
			std::function<void(Modio::ErrorCode, std::map<Modio::ModID, Modio::UserSubscriptionListChangeType>)>&);

		template<typename... OtherArgs>
		bool RequireModManagementEnabled(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::IsModManagementEnabled())
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(
								   Modio::make_error_code(Modio::ModManagementError::ModManagementDisabled),
								   (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireModManagementEnabled<>(std::function<void(Modio::ErrorCode)>& Handler);

		template<typename... OtherArgs>
		bool RequireUserNotSubscribed(Modio::ModID IDToCheck,
									  std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::GetUserSubscriptions().Get().count(IDToCheck))
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::ModManagementError::AlreadySubscribed),
												 (OtherArgs {})...);
						   });
				return false;
			}
			else
			{
				return true;
			}
		}
		template bool RequireUserNotSubscribed<>(Modio::ModID ID, std::function<void(Modio::ErrorCode)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidInitParams(const Modio::InitializeOptions& Options,
									std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			bool bInvalidParameter = false;
			if (!Options.GameID.IsValid())
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "GameID {} is Invalid",
											*Options.GameID);
				bInvalidParameter = true;
			}
			else if (!Options.APIKey.IsValid())
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "APIKey {} is Invalid",
											*Options.APIKey);
				bInvalidParameter = true;
			}
			else if (Options.User.size() == 0)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Session ID may not be empty");
				bInvalidParameter = true;
			}

			if (bInvalidParameter)
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
			else
			{
				return true;
			}
		}
		template bool RequireValidInitParams<>(const Modio::InitializeOptions& Options,
											   std::function<void(Modio::ErrorCode)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidServerInitParam(const Modio::ServerInitializeOptions& Options,
										 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			bool bInvalidParameter = false;
			if (Options.ModsDirectory.empty())
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "ModsDirectory is Invalid");
				bInvalidParameter = true;
			}
			else if (Options.Token.empty())
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Server Token is Invalid");
				bInvalidParameter = true;
			}

			if (bInvalidParameter)
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
			else
			{
				return RequireValidInitParams(Options, Handler);
			}
		}
		template bool RequireValidServerInitParam<>(const Modio::ServerInitializeOptions&,
													std::function<void(Modio::ErrorCode)>&);

		template<typename... OtherArgs>
		bool RequireValidEditModParams(const Modio::EditModParams& Params,
									   std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			// clang-format off
			bool bHasValidParameter =
				Params.Visibility.has_value() ||
				Params.Description.has_value() ||
				Params.HomepageURL.has_value() ||
				Params.MaturityRating.has_value() ||
				Params.CommunityOptions.has_value() ||
				Params.MetadataBlob.has_value() ||
				Params.Name.has_value() ||
				Params.NamePath.has_value() ||
				Params.Summary.has_value() ||
				Params.LogoPath.has_value() ||
				Params.MetadataKvp.has_value() ||
				Params.Tags.has_value();
			// clang-format on

			if (!bHasValidParameter)
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
			else
			{
				return true;
			}
		}
		template bool RequireValidEditModParams<Modio::Optional<Modio::ModInfo>>(
			const Modio::EditModParams&, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>&);

		template<typename... OtherArgs>
		bool RequireModIsNotUninstallPending(Modio::ModID ModToSubscribeTo,
											 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			ModCollection& SystemCollection = Modio::Detail::SDKSessionData::GetSystemModCollection();
			if (Modio::Optional<Modio::ModCollectionEntry&> Entry = SystemCollection.GetByModID(ModToSubscribeTo))
			{
				if (Entry->GetModState() == Modio::ModState::UninstallPending)
				{
					ModioAsio::post(
						Modio::Detail::Services::GetGlobalContext().get_executor(),
						[CompletionHandler =
							 std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							CompletionHandler(Modio::make_error_code(Modio::ModManagementError::ModBeingProcessed),
											  (OtherArgs {})...);
						});
					return false;
				}
				else
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
		template bool RequireModIsNotUninstallPending<>(Modio::ModID ModToSubscribeTo,
														std::function<void(Modio::ErrorCode)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidXBoxRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::XboxToken))
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireValidXBoxRefreshEntitlementsExtendedParameters<Modio::Optional<Modio::TransactionRecord>>(
			Modio::EntitlementParams,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>&);
		template bool RequireValidXBoxRefreshEntitlementsExtendedParameters<
			Modio::Optional<Modio::EntitlementConsumptionStatusList>>(
			Modio::EntitlementParams,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>&);
		template bool RequireValidXBoxRefreshEntitlementsExtendedParameters<Modio::Optional<Modio::EntitlementList>>(
			Modio::EntitlementParams, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>&);

		template<typename... OtherArgs>
		bool RequireValidPSNRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::AuthCode) ||
				User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::PsnToken))
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireValidPSNRefreshEntitlementsExtendedParameters<Modio::Optional<Modio::TransactionRecord>>(
			Modio::EntitlementParams User,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>& Handler);
		template bool RequireValidPSNRefreshEntitlementsExtendedParameters<
			Modio::Optional<Modio::EntitlementConsumptionStatusList>>(
			Modio::EntitlementParams User,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>& Handler);
		template bool RequireValidPSNRefreshEntitlementsExtendedParameters<Modio::Optional<Modio::EntitlementList>>(
			Modio::EntitlementParams User,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidOculusExtendedParameters(Modio::AuthenticationParams User,
												  std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if ((User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::Device) &&
				 !User.ExtendedParameters[Modio::Detail::Constants::APIStrings::Device].empty()) &&
				(User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::UserID) &&
				 !User.ExtendedParameters[Modio::Detail::Constants::APIStrings::UserID].empty()) &&
				(User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::Nonce) &&
				 !User.ExtendedParameters[Modio::Detail::Constants::APIStrings::Nonce].empty()))
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireValidOculusExtendedParameters<>(Modio::AuthenticationParams,
															 std::function<void(Modio::ErrorCode)>&);

		template<typename... OtherArgs>
		bool RequireValidGoogleRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::Receipt))
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}
		template bool RequireValidGoogleRefreshEntitlementsExtendedParameters<
			Modio::Optional<Modio::TransactionRecord>>(
			Modio::EntitlementParams,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>&);
		template bool RequireValidGoogleRefreshEntitlementsExtendedParameters<Modio::Optional<Modio::EntitlementList>>(
			Modio::EntitlementParams, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>&);
		template bool RequireValidGoogleRefreshEntitlementsExtendedParameters<
			Modio::Optional<Modio::EntitlementConsumptionStatusList>>(
			Modio::EntitlementParams,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>&);

		template<typename... OtherArgs>
		bool RequireValidMetaRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if ((User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::Device) &&
				 !User.ExtendedParameters[Modio::Detail::Constants::APIStrings::Device].empty()) &&
				(User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::UserID) &&
				 !User.ExtendedParameters[Modio::Detail::Constants::APIStrings::UserID].empty()))
			{
				return true;
			}
			else
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}

		template bool RequireValidMetaRefreshEntitlementsExtendedParameters<Modio::Optional<Modio::TransactionRecord>>(
			Modio::EntitlementParams,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>&);
		template bool RequireValidMetaRefreshEntitlementsExtendedParameters<Modio::Optional<Modio::EntitlementList>>(
			Modio::EntitlementParams, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>&);
		template bool RequireValidMetaRefreshEntitlementsExtendedParameters<
			Modio::Optional<Modio::EntitlementConsumptionStatusList>>(
			Modio::EntitlementParams,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>&);

		template<typename... OtherArgs>
		bool RequireFileExists(Modio::filesystem::path FileToCheck,
							   std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			Modio::Detail::FileService& FileService =
				Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();

			if (FileService.FileExists(FileToCheck) == true)
			{
				return true;
			}

			ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						   CompletionHandler(Modio::make_error_code(Modio::FilesystemError::FileNotFound),
											 (OtherArgs {})...);
					   });
			return false;
		}
		template bool RequireFileExists<Modio::Optional<Modio::ModID>>(
			Modio::filesystem::path FileToCheck,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModID>)>& Handler);
		template bool RequireFileExists<Modio::Optional<Modio::ModInfo>>(
			Modio::filesystem::path FileToCheck,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>& Handler);
		template bool RequireFileExists<>(Modio::filesystem::path FileToCheck,
										  std::function<void(Modio::ErrorCode)>& Handler);

		template<typename... OtherArgs>
		bool RequireModIDNotInTempModSet(const Modio::ModID& ID,
										 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::SDKSessionData::GetTempModCollection().GetByModID(ID).has_value())
			{
				return true;
			}

			ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
											 (OtherArgs {})...);
					   });
			return false;
		}
		template bool RequireModIDNotInTempModSet<>(const Modio::ModID& ModToSubscribeTo,
													std::function<void(Modio::ErrorCode)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidModID(const Modio::ModID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (ID.IsValid())
			{
				return true;
			}
			ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
											 (OtherArgs {})...);
					   });
			return false;
		}
		template bool RequireValidModID<Modio::Optional<Modio::ModInfo>>(
			const Modio::ModID&, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>&);
		template bool RequireValidModID<Modio::Optional<Modio::ModDependencyList>>(
			const Modio::ModID&, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDependencyList>)>&);
		template bool RequireValidModID<Modio::Optional<Modio::TransactionRecord>>(
			const Modio::ModID&, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>&);
		template bool RequireValidModID<Modio::Optional<std::string>>(
			const Modio::ModID&, std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)>&);
		template bool RequireValidModID<>(const Modio::ModID&, std::function<void(Modio::ErrorCode)>&);

		template<typename... OtherArgs>
		bool RequireAllModIDsValid(const std::vector<Modio::ModID>& Mods,
								   std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			bool InvalidMod = false;
			for (const Modio::ModID& ID : Mods)
			{
				if (!ID.IsValid())
				{
					InvalidMod = true;
					break;
				}
			}
			if (InvalidMod)
			{
				ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
			else
			{
				return true;
			}
		}
		template bool RequireAllModIDsValid<>(const std::vector<Modio::ModID>& Mods,
											  std::function<void(Modio::ErrorCode)>&);
		template bool RequireAllModIDsValid<std::set<Modio::ModID>>(
			const std::vector<Modio::ModID>& Mods, std::function<void(Modio::ErrorCode, std::set<Modio::ModID>)>&);

		template<typename... OtherArgs>
		bool RequireValidModCollectionID(const Modio::ModCollectionID& ID,
										 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (ID.IsValid())
			{
				return true;
			}
			ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
											 (OtherArgs {})...);
					   });
			return false;
		}
		template bool RequireValidModCollectionID<>(const Modio::ModCollectionID& ID,
													std::function<void(Modio::ErrorCode)>& Handler);
		template bool RequireValidModCollectionID<Modio::Optional<Modio::ModCollectionInfo>>(
			const Modio::ModCollectionID& ID,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfo>)>& Handler);
		template bool RequireValidModCollectionID<Modio::Optional<Modio::ModInfoList>>(
			const Modio::ModCollectionID& ID,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>& Handler);
		template bool RequireValidModCollectionID<Modio::Optional<std::string>>(
			const Modio::ModCollectionID& ID,
			std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidGameID(const Modio::GameID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (ID.IsValid())
			{
				return true;
			}
			ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
											 (OtherArgs {})...);
					   });
			return false;
		}
		template bool RequireValidGameID<Modio::Optional<Modio::GameInfo>>(
			const Modio::GameID& ID, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfo>)>&);

		template<typename... OtherArgs>
		bool RequireValidUserID(const Modio::UserID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (ID.IsValid())
			{
				return true;
			}
			ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
											 (OtherArgs {})...);
					   });
			return false;
		}
		template bool RequireValidUserID<>(const Modio::UserID&, std::function<void(Modio::ErrorCode)>&);
		template bool RequireValidUserID<>(const Modio::UserID&, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)>&);

		template<typename... OtherArgs>
		bool RequireValidReportParams(const Modio::ReportParams& Params,
									  std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Params.IsResourceIdValid())
			{
				return true;
			}
			ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
											 (OtherArgs {})...);
					   });
			return false;
		}
		template bool RequireValidReportParams<>(const Modio::ReportParams& Params,
												 std::function<void(Modio::ErrorCode)>&);

		template<typename... OtherArgs>
		bool RequireFetchExternalUpdatesNOTRunning(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::SDKSessionData::IsFetchExternalUpdatesRunning())
			{
				Modio::Detail::SDKSessionData::SetFetchExternalUpdatesRunning(true);
				return true;
			}
			ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						   CompletionHandler(Modio::make_error_code(Modio::GenericError::RequestInProgress),
											 (OtherArgs {})...);
					   });
			return false;
		}
		template bool RequireFetchExternalUpdatesNOTRunning<>(std::function<void(Modio::ErrorCode)>&);

	} // namespace Detail
} // namespace Modio