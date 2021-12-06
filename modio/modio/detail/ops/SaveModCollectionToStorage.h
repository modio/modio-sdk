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
#include "modio/core/ModioServices.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/file/ModioFile.h"
#include "modio/file/ModioFileService.h"
#include "modio/detail/AsioWrapper.h"
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
						nlohmann::json ModCollectionData =
							nlohmann::json::object({Modio::Detail::SDKSessionData::GetSystemModCollection()});
						ModCollectionData["version"] = 1;
						auto teststring = ModCollectionData.dump();
						DestinationFile = std::make_unique<Modio::Detail::File>(
							Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
									.LocalMetadataFolder() /
								"state.json",
							true);
						DataBuffer = std::make_unique<Modio::Detail::Buffer>(teststring.size(), 1024 * 4);
						std::copy(teststring.begin(), teststring.end(), DataBuffer->begin());
					}

					yield DestinationFile->WriteAsync(std::move(*DataBuffer), std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}
					else
					{
						Self.complete({});
					}
				}
			}

		private:
			asio::coroutine CoroutineState;
			std::unique_ptr<Modio::Detail::File> DestinationFile;
			std::unique_ptr<Modio::Detail::Buffer> DataBuffer;
		};

		template<typename SaveModCollectionCallback>
		auto SaveModCollectionToStorageAsync(SaveModCollectionCallback&& OnSaveComplete)
		{
			return asio::async_compose<SaveModCollectionCallback, void(Modio::ErrorCode)>(
				SaveModCollectionToStorage(), OnSaveComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
