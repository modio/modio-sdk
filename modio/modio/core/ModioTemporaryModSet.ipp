/*
 *  Copyright (C) 2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/core/ModioTemporaryModSet.h"
#endif

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioServices.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/file/ModioFileService.h"

#include <algorithm>

namespace Modio
{
	namespace Detail
	{
		TemporaryModSet::TemporaryModSet(std::vector<Modio::ModID> vectorModIds)
		{
			Add(vectorModIds);
		}

		void TemporaryModSet::Add(std::vector<Modio::ModID> vectorModIds)
		{
			for (Modio::ModID Id : vectorModIds)
			{
				if (std::find(ModIds.begin(), ModIds.end(), Id) == ModIds.end() &&
					std::find(TempModIdsToInstall.begin(), TempModIdsToInstall.end(), Id) == TempModIdsToInstall.end())
				{
					if (Modio::Detail::SDKSessionData::GetSystemModCollection().GetByModID(Id).has_value())
					{
						ModIds.push_back(Id);
					}
					else
					{
						TempModIdsToInstall.push_back(Id);
					}
				}
			}
		}

		void TemporaryModSet::Remove(std::vector<Modio::ModID> vectorModIds)
		{
			for (ModID Id : vectorModIds)
			{
				auto iterModID = std::find(ModIds.begin(), ModIds.end(), Id);
				if (iterModID != ModIds.end())
				{
					ModIds.erase(iterModID);

					if (Modio::Detail::SDKSessionData::GetTempModCollection()
														   .GetByModID(Id)
														   .has_value())
					{
						Modio::Detail::SDKSessionData::GetTempModCollection().GetByModID(Id)->SetModState(
							ModState::UninstallPending);
					}
				}
			}
		}

		TemporaryModSet::~TemporaryModSet()
		{
			Remove(ModIds);
		}

		std::vector<Modio::ModID> TemporaryModSet::GetTempModIdsToInstall() const
		{
			return TempModIdsToInstall;
		}

		std::vector<Modio::ModID> TemporaryModSet::GetModIds() const
		{
			return ModIds;
		}

		bool TemporaryModSet::ContainsModId(Modio::ModID Id) const
		{
			return std::find(ModIds.begin(), ModIds.end(), Id) != ModIds.end() ||
				   std::find(TempModIdsToInstall.begin(), TempModIdsToInstall.end(), Id) != TempModIdsToInstall.end();
		}

		void TemporaryModSet::AddModInfoToTempModCollection(Modio::ModInfo ModInfoData)
		{
			auto iterModID = std::find(TempModIdsToInstall.begin(), TempModIdsToInstall.end(), ModInfoData.ModId);
			if (std::find(ModIds.begin(), ModIds.end(), ModInfoData.ModId) == ModIds.end())
			{
				ModIds.push_back(ModInfoData.ModId);
				TempModIdsToInstall.erase(iterModID);

				Modio::Detail::SDKSessionData::GetTempModCollection().AddOrUpdateMod(
					ModInfoData, Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
									 .MakeTempModPath(ModInfoData.ModId)
									 .u8string());
			}
			else
			{
				Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::ModManagement,
											"Mod {} already in TempModSet ", ModInfoData.ModId);
			}
		}
	}
} // namespace Modio
