/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioSDKMultiplayerLibrary.h"
#include "modio/detail/ops/mod/ListAllModsOp.h"
#include "modio/detail/ops/monetization/FetchUserPurchasesOp.h"
#include <algorithm>
#include <asio/coroutine.hpp>
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class InstallOrUpdateServerModsOp
		{
		private:
			asio::coroutine CoroutineState;
			std::vector<ModID> Mods;
			Modio::Optional<Modio::ModInfoList> ValidFoundMods;

		public:
			InstallOrUpdateServerModsOp(std::vector<ModID> InMods) : Mods(InMods) {};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, Modio::Optional<Modio::ModInfoList> ModList = {})
			{
				if (Modio::ModioServer::Get().CachedServerInitializationOptions.Mods.size() == 0 &&
					Mods.empty())
				{
					Self.complete({});
					return;
				}

				// We want to get all mods, both the cached initalization ones as well as the ones passed to this
				// function
				std::vector<Modio::ModID> ModsToGet(Mods.begin(), Mods.end());
				ModsToGet.insert(ModsToGet.end(), ModioServer::Get().CachedServerInitializationOptions.Mods.begin(),
								 ModioServer::Get().CachedServerInitializationOptions.Mods.end());

				Modio::FilterParams Filter;
				Filter.MatchingIDs(ModsToGet);
				Filter.RevenueType(Modio::FilterParams::RevenueFilterType::FreeAndPaid);

				reenter(CoroutineState)
				{
					yield Modio::Detail::ListAllModsAsync(Filter, std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}
					else
					{
						// It is possible for the call to succeed but also return no mods (invalid IDs)
						// This is not an error per-se, but is considered an error for this call specifically
						if (ModList.value().Size() == 0)
						{
							Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::ModManagement,
														"Install or Update Server Mods returned 0 mods. "
														"Ensure the given mod IDs are valid.");
							Self.complete(Modio::make_error_code(Modio::GenericError::BadParameter));
							return;
						}

						ValidFoundMods = std::move(ModList);

						yield Modio::Detail::FetchUserPurchasesAsync(std::move(Self));

						if (ec)
						{
							// Not neccessarily an error, user may not have a wallet, or purchases.
							Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::ModManagement,
								"Failed to fetch user purchases for mod ownership check. Error: {}", ec.message());

							Self.complete(ec);
							return;
						}
						else
						{
							std::map<Modio::ModID, Modio::ModInfo> UserPurchases = Modio::QueryUserPurchasedMods();
							std::vector<Modio::ModID> FoundMods;

							for (const Modio::ModInfo& ModInfo : ValidFoundMods.value())
							{
								if (ModInfo.Price > 0 && !UserPurchases.count(ModInfo.ModId))
								{
									Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::ModManagement,
																"Tried to get ModID {} but Server token does not own "
																"it/have priviliges to access it. "
																"Please check your OAuth token or purchase the UGC.",
																ModInfo.ModId);
									continue;
								}

								FoundMods.push_back(ModInfo.ModId);
								Modio::Detail::Logger().Log(
									Modio::LogLevel::Error, Modio::LogCategory::ModManagement,
									"Installing mod {} to path: {}", ModInfo.ModId,
									Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
										.MakeModPath(ModInfo.ModId)
										.u8string());

								Modio::Detail::SDKSessionData::GetSystemModCollection().AddOrUpdateMod(
									ModInfo, Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
												 .MakeModPath(ModInfo.ModId)
												 .u8string());
							}

							// Check if the number of mods we recieved is different to the number of mods we requested
							if (FoundMods.size() != ModsToGet.size())
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
															"Could not get all mods for server.");
								for (const Modio::ModID& Mod : ModsToGet)
								{
									if (std::find(FoundMods.begin(), FoundMods.end(), Mod) == FoundMods.end())
									{
										Modio::Detail::Logger().Log(
											Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
											"Failed to get mod {}, probably an invalid ModID", Mod);
									}
								}
							}

							Self.complete({});
							return;
						}
					}
				}
			}
		};

		template<typename InstallDoneCallback>
		auto InstallOrUpdateServerModsAsync(std::vector<ModID>& Mods, InstallDoneCallback&& OnInstallComplete)
		{
			return asio::async_compose<InstallDoneCallback, void(Modio::ErrorCode)>(
				InstallOrUpdateServerModsOp(Mods), OnInstallComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
