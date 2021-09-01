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

#include "modio/core/ModioServices.h"
#include "modio/userdata/ModioUserDataService.h"
#include "modio/core/ModioLogger.h"
#include "modio/file/ModioFileService.h"
#include "modio/file/ModioFile.h"
#include "modio/core/ModioErrorCode.h"
#include <asio/yield.hpp>
#include <memory>
namespace Modio
{
	namespace Detail
	{
		class InitializeUserDataOp
		{
		public:
			InitializeUserDataOp() {}

			template<typename OperationState>
			void operator()(OperationState& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::User,
													"Loading user data");

						RootPath =
							Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().UserDataFolder();
						if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().FileExists(
								RootPath / "user.json"))
						{
							DestinationFile = std::make_unique<Modio::Detail::File>(RootPath / "user.json", false);

							yield DestinationFile->ReadAsync(DestinationFile->GetFileSize(), DataBuffer,
															  std::move(Self));
							if (ec)
							{
								Self.complete(ec);
								return;
							}
							else
							{
								if (DataBuffer.size() > 0)
								{
									Modio::Detail::Buffer CollatedBuffer(DataBuffer.size());
									Modio::Detail::BufferCopy(CollatedBuffer, DataBuffer);
									if (Modio::Detail::SDKSessionData::DeserializeUserDataFromBuffer(
											std::move(CollatedBuffer)))
									{
										Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::User,
																	"User Data Loaded");
									}
									else
									{
										Modio::Detail::Logger().Log(
											Modio::LogLevel::Trace, Modio::LogCategory::User,
											"User Data file present but not valid. Initializing to default state");
									}
								}
								else
								{
									Modio::Detail::Logger().Log(
										Modio::LogLevel::Info, Modio::LogCategory::User,
										"User data file not readable or zero-length. Initializing to default state");
								}
								Self.complete({});
								return;
							}
						}
						else
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::User,
														"User data file not found. Initializing to default state");
						}
					}

					Self.complete({});
					return;
				}
			}

		private:
			asio::coroutine CoroutineState;

			std::unique_ptr<Modio::Detail::File> DestinationFile;
			Modio::Detail::DynamicBuffer DataBuffer;
			Modio::filesystem::path RootPath;
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>