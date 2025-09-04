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
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/ModioSDK.h"
#include <algorithm>
#include <asio/coroutine.hpp>
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class RegisterClientModsWithServerOp
		{
		private:
			asio::coroutine CoroutineState;
			std::vector<ModID> Mods;

		public:
			RegisterClientModsWithServerOp(std::vector<ModID> InMods) : Mods(InMods) {};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, Modio::Optional<Modio::ModInfoList> ModList = {})
			{
				// Early out if the  mod list is empty
				if (Mods.empty())
				{
					Self.complete({}, ModioServer::Get().GetClientMods());
					return;
				}

				Modio::FilterParams Filter;
				Filter.MatchingIDs(Mods);
				Filter.RevenueType(Modio::FilterParams::RevenueFilterType::FreeAndPaid);

				reenter(CoroutineState)
				{
					yield Modio::Detail::ListAllModsAsync(Filter, std::move(Self));
					if (ec)
					{
						Self.complete(ec, ModioServer::Get().GetClientMods());
						return;
					}
					else
					{
						if (ModList.value().Size() == Mods.size())
						{
							ModioServer::Get().AddClientMods(Mods);
						}
						else
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Core,
								"Could not get all client mods for server.");

							if (ModList.value().Size() == 0)
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Core,
															"Found no mods with given IDs.");
								Self.complete({}, ModioServer::Get().GetClientMods());
								return;
							}

							for (auto it = Mods.begin(); it < Mods.end();)
							{
								if (std::find_if(ModList.value().begin(), ModList.value().end(),
									[Id = *it](const Modio::ModInfo& Info)
									{
										return Id == Info.ModId;
									}) == ModList.value().end())
								{
									Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Core,
																"Failed to get mod {}, probably an invalid ModID",
																*it);
									it = Mods.erase(it);
								}
								else
								{
									++it;
								}
							}

							ModioServer::Get().AddClientMods(Mods);
						}

						// Upon receiving this callback the developer should then pass the whole TempModSet to all
						// connected clients.
						Self.complete({}, ModioServer::Get().GetClientMods());
						return;
					}
				}
			}
		};

		template<typename RegisterDoneCallback>
		auto RegisterClientModsWithServerAsync(std::vector<ModID>& Mods, RegisterDoneCallback&& OnRegisterComplete)
		{
			return asio::async_compose<RegisterDoneCallback, void(Modio::ErrorCode, std::set<Modio::ModID>)>(
				RegisterClientModsWithServerOp(Mods), OnRegisterComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
