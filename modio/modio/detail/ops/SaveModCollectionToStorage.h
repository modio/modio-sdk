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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/file/ModioFile.h"
#include "modio/file/ModioFileService.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class SaveModCollectionToStorage : public Modio::Detail::BaseOperation<SaveModCollectionToStorage>
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					{
						MODIO_PROFILE_SCOPE(SerializeModCollection);
						nlohmann::json ModCollectionData = []() {
							auto Lock = Modio::Detail::SDKSessionData::GetReadLock();
							return nlohmann::json::object({Modio::Detail::SDKSessionData::GetSystemModCollection()});
						}();

						ModCollectionData["version"] = 1;
						auto teststring = ModCollectionData.dump();

						DestinationFilePath = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
												  .LocalMetadataFolder() /
											  "state.json";

						// Make temporary file with new state data
						TempFilePath = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
										   .LocalMetadataFolder() /
									   "state.json.tmp";
						TempFile = std::make_unique<Modio::Detail::File>(TempFilePath,
																		 Modio::Detail::FileMode::ReadWrite, true);
						DataBuffer = std::make_unique<Modio::Detail::Buffer>(teststring.size(), 1024 * 4);
						std::copy(teststring.begin(), teststring.end(), DataBuffer->begin());
					}

					// Write new state data to state.json.tmp
					yield TempFile->WriteAsync(std::move(*DataBuffer), std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// Close file to perform MoveAndOverwriteFile()
					TempFile.reset();

					// Updates state.json with the contents of state.json.tmp, and deletes state.json.tmp
					if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().MoveAndOverwriteFile(
							TempFilePath, DestinationFilePath))
					{
						Self.complete({});
						return;
					}
					else
					{
						Self.complete(Modio::make_error_code(Modio::FilesystemError::WriteError));
						return;
					}
				}
			}

		private:
			ModioAsio::coroutine CoroutineState {};
			Modio::filesystem::path DestinationFilePath {};
			Modio::filesystem::path TempFilePath {};
			std::unique_ptr<Modio::Detail::File> TempFile {};
			std::unique_ptr<Modio::Detail::Buffer> DataBuffer {};
		};

		template<typename SaveModCollectionCallback>
		auto SaveModCollectionToStorageAsync(SaveModCollectionCallback&& OnSaveComplete)
		{
			return ModioAsio::async_compose<SaveModCollectionCallback, void(Modio::ErrorCode)>(
				SaveModCollectionToStorage(), OnSaveComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
