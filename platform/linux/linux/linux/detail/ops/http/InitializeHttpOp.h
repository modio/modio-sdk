/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once

#include "linux/HttpSharedState.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ssl.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/file/ModioFile.h"
#include "modio/file/ModioFileService.h"
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
					if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().FileExists(
							"/etc/ssl/certs/ca-certificates.crt"))
					{
						mbedtls_x509_crt_init(&SharedState->CACertificates);
						int CertParseResult = 0;

						if ((CertParseResult = mbedtls_x509_crt_parse_file(&SharedState->CACertificates,
																		   "/etc/ssl/certs/ca-certificates.crt")) < 0)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"Could not parse root certificates from "
														"/etc/ssl/certs/ca-certificates.crt, error {}",
														CertParseResult * -1);
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
							"Could not load root certificate list from /etc/ssl/certs/ca-certificates.crt");
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
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
