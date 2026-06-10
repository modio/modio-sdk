/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */
#pragma once
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioDefaultRequestParameters.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioTemporaryModSet.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/modmanagement/CopyTempModToInstallDirOp.h"
#include "modio/detail/ops/modmanagement/UninstallMod.h"
#include "modio/detail/ModioStringHelpers.h"
#include "modio/file/ModioFileService.h"

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class PromoteTempModOp
		{
			struct PromoteTempModImpl
			{
				Modio::ModID ModId {};
				bool IncludeDependencies = false;
				Modio::Detail::FileService& PlatformFileService;
				Modio::filesystem::path TemporaryModPath;
				Modio::filesystem::path SubscribedModPath;
			};

			Modio::StableStorage<PromoteTempModImpl> Impl;
			ModioAsio::coroutine CoroutineState {};

		public:
			PromoteTempModOp(Modio::ModID ModID, bool IncludeDependencies)
			{
				Impl = std::make_shared<PromoteTempModImpl>(
					PromoteTempModImpl {std::move(ModID),
										std::move(IncludeDependencies),
										Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>(),
										{},
										{}});
			}
			template<typename CoroType>

			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
				{
					Modio::Detail::SDKSessionData::InvalidateOAuthToken();
					Self.complete(ec);
					return;
				}

				reenter(CoroutineState)
				{
					{
						Impl->TemporaryModPath =
							Impl->PlatformFileService.GetTempModRootInstallationPath() / std::to_string(Impl->ModId);

						Impl->SubscribedModPath =
							Impl->PlatformFileService.GetModRootInstallationPath() / std::to_string(Impl->ModId);
					}

					yield Modio::Detail::CopyTempModToInstallDirAsync(Impl->TemporaryModPath, Impl->SubscribedModPath,
																	  std::move(Self));

					if (ec)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::ModManagement,
													"Failed to copy temp mod {} from {} to {}: {}", Impl->ModId,
													Modio::ToModioString(Impl->TemporaryModPath.u8string()),
													Modio::ToModioString(Impl->SubscribedModPath.u8string()), ec.message());
						Self.complete(ec);
						return;
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
												"Copied temp mod {} to permanent path {}", Impl->ModId,
												Modio::ToModioString(Impl->SubscribedModPath.u8string()));

					if (Modio::Detail::SDKSessionData::GetTemporaryModSet() &&
						Modio::Detail::SDKSessionData::GetTemporaryModSet()->ContainsModId(Impl->ModId))
					{
						Modio::Detail::SDKSessionData::GetTemporaryModSet()->Remove({Impl->ModId});
						Modio::Detail::SDKSessionData::GetTempModCollection().RemoveMod(Impl->ModId);

						yield Modio::Detail::UninstallModAsync(Impl->ModId, std::move(Self), false, true);

						if (ec)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
														"Failed to uninstall temp mod {} after promotion: {}",
														Impl->ModId, ec.message());
							Self.complete(ec);
							return;
						}
					}

					Self.complete({});
					return;
				}
			}
		};
		template<typename PromoteCompleteCallback>
		void PromoteTempModAsync(Modio::ModID ModToPromote, bool IncludeDependencies,
								 PromoteCompleteCallback&& OnPromoteComplete)
		{
			return ModioAsio::async_compose<PromoteCompleteCallback, void(Modio::ErrorCode)>(
				PromoteTempModOp(ModToPromote, IncludeDependencies), OnPromoteComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail

} // namespace Modio

#include <asio/unyield.hpp>