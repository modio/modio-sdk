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
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/file/ModioFile.h"
#include "modio/file/ModioFileService.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SaveUserDataToStorageOp
		{
		public:
			SaveUserDataToStorageOp()
			{
				LocalState = std::make_shared<Impl>();

				LocalState->UserDataBuffer = std::make_unique<Modio::Detail::Buffer>(
					Modio::Detail::SDKSessionData::SerializeUserData());
			}

			SaveUserDataToStorageOp(SaveUserDataToStorageOp&& Other)
				: CoroutineState(std::move(Other.CoroutineState)),
				  LocalState(std::move(Other.LocalState))
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					LocalState->UserDataFile = std::make_unique<Modio::Detail::File>(
						Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().UserDataFolder() /
							"user.json",
						true);
					yield LocalState->UserDataFile->WriteAsync(std::move(*LocalState->UserDataBuffer), std::move(Self));

					LocalState->UserDataFile.reset();

					Self.complete(ec);
					return;
				}
			}

		private:
			asio::coroutine CoroutineState;
			struct Impl
			{
				std::unique_ptr<Modio::Detail::File> UserDataFile;
				std::unique_ptr<Modio::Detail::Buffer> UserDataBuffer;
			};

			Modio::StableStorage<Impl> LocalState;
		};
#include <asio/unyield.hpp>

	} // namespace Detail
} // namespace Modio
