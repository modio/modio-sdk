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

#include "android/HttpSharedState.h"
#include "modio/detail/MbedtlsWrapper.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/file/ModioFile.h"
#include "modio/file/ModioFileService.h"
#include "modio/detail/FilesystemWrapper.h"
#include <memory>
#include <string>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class InitializeHttpOp
		{
		public:
			InitializeHttpOp(std::string UserAgentString, std::shared_ptr<Modio::Detail::HttpSharedState> SharedState)
				: SharedState(SharedState)
			{
				this->SharedState->UserAgentString = UserAgentString;
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					{
						Modio::ErrorCode TLSInitStatus = SharedState->Initialize();
						if (TLSInitStatus)
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::HttpNotInitialized));
							return;
						}
					}
					
					Modio::filesystem::path certificateFilePath =
						Modio::Detail::AndroidContextService::Get().GetJavaClassModio()->GetCertificatePath().c_str();

					const std::string certFilePathStr = certificateFilePath.string();
					 
					if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().FileExists(certificateFilePath))
					{
						mbedtls_x509_crt_init(&SharedState->CACertificates);
						int CertParseResult = 0;

						if ((CertParseResult = mbedtls_x509_crt_parse_file(&SharedState->CACertificates,
																		   certFilePathStr.c_str())) < 0)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"Could not parse root certificates from "
														"{0}, error {1}",
														certFilePathStr.c_str(), CertParseResult * -1);
							Self.complete(Modio::make_error_code(Modio::HttpError::HttpNotInitialized));
							return;
						}
						else
						{
							mbedtls_ssl_conf_ca_chain(&SharedState->SSLConfiguration, &SharedState->CACertificates,
													  nullptr);
							Self.complete({});
							return;
						}
						
					}
					else
					{
						Modio::Detail::Logger().Log(
							Modio::LogLevel::Error, Modio::LogCategory::Http,
							"Could not load root certificate list from {}", certFilePathStr.c_str());
						Self.complete(Modio::make_error_code(Modio::HttpError::HttpNotInitialized));
						return;
					}

				}
			}

		private:
			Modio::Detail::DynamicBuffer CertificateBuffer;
			std::unique_ptr<Modio::Detail::File> CertificateFile;
			asio::coroutine CoroutineState;
			std::shared_ptr<HttpSharedState> SharedState;
			Modio::filesystem::path CertificatesFolder;
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
