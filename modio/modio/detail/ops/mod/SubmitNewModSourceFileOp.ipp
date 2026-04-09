/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/ops/mod/SubmitNewModSourceFileOp.h"
#endif

namespace Modio
{
	namespace Detail
	{

			SubmitNewModSourceFileOp::SubmitNewModSourceFileOp(Modio::ModID ModID, Modio::CreateSourceFileParams Params)
				: CurrentModID(ModID),
				  CurrentModParams(Params)
			{
				ModRootDirectory = Params.RootDirectory;
				ArchivePath = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
								  .MakeTempFilePath(fmt::format("modfile_{}.zip", ModID))
								  .value_or("");
				ProgressInfo = Modio::Detail::SDKSessionData::StartModDownloadOrUpdate(CurrentModID);
				FileHash = std::make_shared<uint64_t>(0ULL);
			}
			Modio::Detail::HttpRequestParams SubmitNewModSourceFileOp::CreateSourceRequestParams(
				HttpRequestParams AddRequest, Modio::ModID ModID,
																	   Modio::CreateSourceFileParams Params)
			{
				Modio::Detail::HttpRequestParams RequestParams =
					AddRequest.SetModID(ModID)
						.AppendPayloadValue("version", Params.Version)
						.AppendPayloadValue("changelog", Params.Changelog)
						.AppendPayloadValue("metadata", Params.MetadataBlob);

				if (Params.Platforms)
				{
					// sort Platforms vector and remove duplicates
					std::sort(Params.Platforms.value().begin(), Params.Platforms.value().end());
					auto Last = std::unique(Params.Platforms.value().begin(), Params.Platforms.value().end());
					Params.Platforms.value().erase(Last, Params.Platforms.value().end());

					// loop through vector to append appropriate value to request
					std::size_t i = 0;
					for (const Modio::ModfilePlatform Platform : *Params.Platforms)
					{
						switch (Platform)
						{
							case (Modio::ModfilePlatform::Windows):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Windows);
								break;
							case (Modio::ModfilePlatform::Mac):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Mac);
								break;
							case (Modio::ModfilePlatform::Linux):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Linux);
								break;
							case (Modio::ModfilePlatform::Android):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Android);
								break;
							case (Modio::ModfilePlatform::iOS):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::iOS);
								break;
							case (Modio::ModfilePlatform::XboxOne):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::XboxOne);
								break;
							case (Modio::ModfilePlatform::XboxSeriesX):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i),
									Modio::Detail::Constants::PlatformNames::XboxSeriesX);
								break;
							case (Modio::ModfilePlatform::PS4):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::PS4);
								break;
							case (Modio::ModfilePlatform::PS5):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::PS5);
								break;
							case (Modio::ModfilePlatform::Switch):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Switch);
								break;
							case (Modio::ModfilePlatform::Oculus):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Oculus);
								break;
							case (Modio::ModfilePlatform::WindowsServer):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i),
									Modio::Detail::Constants::PlatformNames::WindowsServer);
								break;
							case (Modio::ModfilePlatform::LinuxServer):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i),
									Modio::Detail::Constants::PlatformNames::LinuxServer);
								break;
							case (Modio::ModfilePlatform::Source):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Source);
								break;
							default:
								Modio::Detail::Logger().Log(
									LogLevel::Warning, LogCategory::File,
									"Platform {} does not match any Modio::ModfilePlatform values. "
									"Unable to append to SubmitNewModFileForMod request.",
									Platform);
						}
						i++;
					}
				}

				return RequestParams;
			}
	} // namespace Detail
} // namespace Modio