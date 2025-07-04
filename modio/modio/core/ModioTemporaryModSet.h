/*
 *  Copyright (C) 2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/core/ModioModCollectionEntry.h"



namespace Modio
{
	namespace Detail
	{
		/// @docpublic
		/// @brief Class to manage Temp Mod
		class TemporaryModSet
		{
		private:
			/// @brief vector of ModIds who are in the Temp/System Collection
			std::vector<ModID> ModIds {};

			/// @brief vector of ModIds to collect ModInfo data before adding it to the TempModCollection
			std::vector<ModID> TempModIdsToInstall {};

		public:
			/// @docinternal
			/// @brief Default constructor
			MODIO_IMPL TemporaryModSet() = default;

			MODIO_IMPL TemporaryModSet(std::vector<Modio::ModID> vectorModIds);

			MODIO_IMPL ~TemporaryModSet();

			MODIO_IMPL void Add(std::vector<Modio::ModID> vectorModIds);

			MODIO_IMPL void Remove(std::vector<Modio::ModID> vectorModIds);

			MODIO_IMPL std::vector<ModID> GetTempModIdsToInstall() const;

			MODIO_IMPL std::vector<ModID> GetModIds() const;

			MODIO_IMPL bool ContainsModId(Modio::ModID Id) const;

			MODIO_IMPL void AddModInfoToTempModCollection(Modio::ModInfo ModInfoData);
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioTemporaryModSet.ipp"
#endif