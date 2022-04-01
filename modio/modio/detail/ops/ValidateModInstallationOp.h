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
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		/// @brief Class that validates the installation of a ModCollectionEntry by checking that the specified
		/// directory exists, and contains data.
		class ValidateModInstallationOp
		{
			asio::coroutine CoroutineState;
			Modio::ModCollectionEntry Entry;

		public:
			ValidateModInstallationOp(Modio::ModCollectionEntry Entry) : Entry(Entry) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					// If the directory does not exist
					if (!Modio::filesystem::exists(Entry.GetPath(), ec))
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::ModManagement,
													"Directory {} for installed mod with ID {} does not exist",
													Entry.GetPath().u8string(), Entry.GetID());

						Self.complete(Modio::make_error_code(Modio::ModValidationError::ModDirectoryNotFound));
						return;
					}
					else if (Modio::filesystem::is_empty(Entry.GetPath(), ec)) // If the directory is empty
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::ModManagement,
													"Directory {} for installed mod with ID {} is empty",
													Entry.GetPath().u8string(), Entry.GetID());

						Self.complete(Modio::make_error_code(Modio::ModValidationError::NoFilesFoundForMod));
						return;
					}
					else if (ec) // Otherwise we got an error trying to check the directory
					{
						Modio::Detail::Logger().Log(
							Modio::LogLevel::Error, Modio::LogCategory::ModManagement,
							"Received error code {}, message {} while validating installed mod with ID {}", ec.value(),
							ec.message().c_str(), Entry.GetID());
						Self.complete(ec);
					}

					Self.complete({});
					return;
				}
			}
		};

		template<typename CompletionToken>
		void ValidateModInstallationAsync(Modio::ModCollectionEntry& Entry, CompletionToken&& Callback)
		{
			return asio::async_compose<CompletionToken, void(Modio::ErrorCode)>(
				ValidateModInstallationOp(Entry), Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>