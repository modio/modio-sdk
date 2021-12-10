/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once

#include "http/HttpRequestImplementation.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "modio/core/ModioErrorCode.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
		struct HttpSharedState
		{
			bool bCloseRequested = false;

			mbedtls_ssl_config SSLConfiguration;
			mbedtls_entropy_context EntropyContext;
			mbedtls_ctr_drbg_context RandomContext;
			mbedtls_x509_crt CACertificates;
			std::string UserAgentString; 
			Modio::ErrorCode Initialize()
			{
				mbedtls_entropy_init(&EntropyContext);
				mbedtls_ctr_drbg_init(&RandomContext);
				mbedtls_ssl_config_init(&SSLConfiguration);

				if (mbedtls_ssl_config_defaults(&SSLConfiguration, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
												MBEDTLS_SSL_PRESET_DEFAULT))
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
												"Could not set default TLS configuration");
					return Modio::make_error_code(Modio::HttpError::SecurityConfigurationInvalid);
				}

				if (mbedtls_ctr_drbg_seed(&RandomContext, mbedtls_entropy_func, &EntropyContext, NULL, 0) != 0)
				{
					mbedtls_ctr_drbg_free(&RandomContext);
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
												"Could not seed RNG for TLS");
					return Modio::make_error_code(Modio::HttpError::SecurityConfigurationInvalid);
				}

				mbedtls_ssl_conf_rng(&SSLConfiguration, mbedtls_ctr_drbg_random, &RandomContext);
				mbedtls_ssl_conf_authmode(&SSLConfiguration, MBEDTLS_SSL_VERIFY_REQUIRED);
				return {};
			}

			void InitializeRequest(std::shared_ptr<HttpRequestImplementation> Request, Modio::ErrorCode& ec)
			{
				// UserAgentString will be set by InitializeHttpOp using HttpSharedState object
				Request->GetParameters().SetUserAgentOverride(UserAgentString);

				// will need to open the socket etc here using the mbedtls wrappers maybe
				mbedtls_ssl_init(&Request->SSLContext);
				mbedtls_ssl_setup(&Request->SSLContext, &SSLConfiguration);
				mbedtls_ssl_set_hostname(&Request->SSLContext, Request->Parameters.GetServerAddress().c_str());
				mbedtls_net_init(&Request->Socket);
				if (mbedtls_net_connect(&Request->Socket, Request->Parameters.GetServerAddress().c_str(), "443",
										MBEDTLS_NET_PROTO_TCP))
				{
					ec = Modio::make_error_code(Modio::HttpError::CannotOpenConnection);
					return;
				}
				else
				{
					mbedtls_ssl_set_bio(&Request->SSLContext, &Request->Socket, mbedtls_net_send, mbedtls_net_recv,
										nullptr);
				}
			}

			void Close()
			{
				bCloseRequested = true;
			}

			bool IsClosing()
			{
				return bCloseRequested;
			}
		};
	} // namespace Detail
} // namespace Modio