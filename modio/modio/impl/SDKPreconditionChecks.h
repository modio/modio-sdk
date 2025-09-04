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

#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioServerInitializeOptions.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioMetricsService.h"
#include "modio/core/ModioServices.h"
#include "modio/core/ModioEditModParams.h"
#include "modio/core/ModioReportParams.h"
#include "modio/file/ModioFileService.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/serialization/ModioEntitlementConsumptionStatusListSerialization.h"

 /// @brief These are templated helper functions - they take in the user-provided completion handler, check some
 /// precondition, and if the precondition check fails they :
 /// * Post the user completion handler to the io_context with the specified error AND
 /// * return false
 ///
 /// This allows the calling function to skip over the initiation of the asynchronous operation if any of the checks
 /// failed, knowing that the completion handler has already been queued with the correct error code, so the caller
 /// doesn't need to care which of the preconditions failed.

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
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::GenericError::SDKNotInitialized),
							(OtherArgs{})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireUserIsAuthenticated(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::GetAuthenticatedUser().has_value())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::UserDataError::InvalidUser),
							(OtherArgs{})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireUserIsNOTAuthenticated(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			// Note that GetAuthenticationToken() checks token state, and only returns valid tokens. No need to re-check
			// state here.
			if (!Modio::Detail::SDKSessionData::GetAuthenticationToken().has_value())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::UserAuthError::AlreadyAuthenticated),
							(OtherArgs{})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsInitialized(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().IsInitialized())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::MetricsError::SessionNotInitialized),
							(OtherArgs{})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsNOTInitialized(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().IsInitialized())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::MetricsError::SessionAlreadyInitialized),
							(OtherArgs{})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsActive(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().GetSessionIsActive())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::MetricsError::SessionIsNotActive),
							(OtherArgs{})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsNOTActive(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().GetSessionIsActive())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::MetricsError::SessionIsActive),
							(OtherArgs{})...);
					});
				return false;
			}
		}

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
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
							(OtherArgs{})...);
					});
				return false;
			}
			else
			{
				return true;
			}
		}

		template<typename... OtherArgs>
		bool RequireNotRateLimited(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::SDKSessionData::IsRateLimited())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::HttpError::RateLimited),
							(OtherArgs{})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireModManagementEnabled(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::IsModManagementEnabled())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(
							Modio::make_error_code(Modio::ModManagementError::ModManagementDisabled),
							(OtherArgs{})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireUserNotSubscribed(Modio::ModID IDToCheck,
			std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::GetUserSubscriptions().Get().count(IDToCheck))
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::ModManagementError::AlreadySubscribed),
							(OtherArgs{})...);
					});
				return false;
			}
			else
			{
				return true;
			}
		}

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
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
							(OtherArgs{})...);
					});
				return false;
			}
			else
			{
				return true;
			}
		}

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
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
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
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
							(OtherArgs{})...);
					});
				return false;
			}
			else
			{
				return true;
			}
		}
		template<typename... OtherArgs>
		bool RequireModIsNotUninstallPending(Modio::ModID ModToSubscribeTo,
			std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			ModCollection& SystemCollection = Modio::Detail::SDKSessionData::GetSystemModCollection();
			if (Modio::Optional<Modio::ModCollectionEntry&> Entry = SystemCollection.GetByModID(ModToSubscribeTo))
			{
				if (Entry->GetModState() == Modio::ModState::UninstallPending)
				{
					asio::post(
						Modio::Detail::Services::GetGlobalContext().get_executor(),
						[CompletionHandler =
						std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							CompletionHandler(Modio::make_error_code(Modio::ModManagementError::ModBeingProcessed),
								(OtherArgs{})...);
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

		template<typename... OtherArgs>
		bool RequireValidXBoxRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>,
				OtherArgs...)>& Handler)
		{
			if (User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::XboxToken))
			{
				return true;
			}
			else
			{
				asio::post(
					Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler = std::forward<std::function<void(
						Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>, OtherArgs...)>>(
							Handler)]() mutable {
								CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter), {},
									(OtherArgs{})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireValidPSNRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>,
				OtherArgs...)>& Handler)
		{
			if (User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::AuthCode))
			{
				return true;
			}
			else
			{
				asio::post(
					Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler = std::forward<std::function<void(
						Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>, OtherArgs...)>>(
							Handler)]() mutable {
								CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter), {},
									(OtherArgs{})...);
					});
				return false;
			}
		}

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
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler =
					std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
							(OtherArgs{})...);
					});
				return false;
			}
		}
		
		template<typename... OtherArgs>
		bool RequireValidGoogleRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>,
							   OtherArgs...)>& Handler)
		{
			if (User.ExtendedParameters.count(Modio::Detail::Constants::APIStrings::Receipt))
			{
				return true;
			}
			else
			{
				asio::post(
					Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler = std::forward<std::function<void(
						 Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>, OtherArgs...)>>(
						 Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter), {},
										  (OtherArgs {})...);
					});
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireValidMetaRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>,
							   OtherArgs...)>& Handler)
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
				asio::post(
					Modio::Detail::Services::GetGlobalContext().get_executor(),
					[CompletionHandler = std::forward<std::function<void(
						 Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>, OtherArgs...)>>(
						 Handler)]() mutable {
						CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter), {},
										  (OtherArgs {})...);
					});
				return false;
			}
		}

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

			asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
				[CompletionHandler =
				std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
					CompletionHandler(Modio::make_error_code(Modio::FilesystemError::FileNotFound),
						(OtherArgs{})...);
				});
			return false;
		}

		template<typename... OtherArgs>
		bool RequireModIDNotInTempModSet(const Modio::ModID& ID,
			std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::SDKSessionData::GetTempModCollection().GetByModID(ID).has_value())
			{
				return true;
			}

			asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
				[CompletionHandler =
				std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
					CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
						(OtherArgs{})...);
				});
			return false;
		}

		template<typename... OtherArgs>
		bool RequireValidModID(const Modio::ModID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (ID.IsValid())
			{
				return true;
			}
			asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
				[CompletionHandler =
				std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
					CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
						(OtherArgs{})...);
				});
			return false;
		}

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
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
					CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter), (OtherArgs {})...);
					});
				return false;
			}
			else
			{
				return true;
			}
		}

		template<typename... OtherArgs>
		bool RequireValidModCollectionID(const Modio::ModCollectionID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (ID.IsValid())
			{
				return true;
			}
			asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
					   [CompletionHandler =
							std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
						   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
											 (OtherArgs {})...);
					   });
			return false;
		}

		template<typename... OtherArgs>
		bool RequireValidGameID(const Modio::GameID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (ID.IsValid())
			{
				return true;
			}
			asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
				[CompletionHandler =
				std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
					CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
						(OtherArgs{})...);
				});
			return false;
		}
		template<typename... OtherArgs>
		bool RequireValidUserID(const Modio::UserID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (ID.IsValid())
			{
				return true;
			}
			asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
				[CompletionHandler =
				std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
					CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
						(OtherArgs{})...);
				});
			return false;
		}
		template<typename... OtherArgs>
		bool RequireValidReportParams(const Modio::ReportParams& Params,
			std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Params.IsResourceIdValid())
			{
				return true;
			}
			asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
				[CompletionHandler =
				std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
					CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
						(OtherArgs{})...);
				});
			return false;
		}

		template<typename... OtherArgs>
		bool RequireFetchExternalUpdatesNOTRunning(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::SDKSessionData::IsFetchExternalUpdatesRunning())
			{
				Modio::Detail::SDKSessionData::SetFetchExternalUpdatesRunning(true);
				return true;
			}
			asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
				[CompletionHandler =
				std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
					CompletionHandler(Modio::make_error_code(Modio::GenericError::RequestInProgress),
						(OtherArgs{})...);
				});
			return false;
		}
	} // namespace Detail
} // namespace Modio