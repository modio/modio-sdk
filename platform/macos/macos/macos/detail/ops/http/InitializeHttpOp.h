/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once

#include "macos/HttpSharedState.h"
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
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					Modio::ErrorCode SharedInitStatus = SharedState->Initialize();
					if (SharedInitStatus)
					{
						Self.complete(Modio::make_error_code(Modio::HttpError::HttpNotInitialized));
						return;
					}

					Self.complete({});
				}
			}

		private:
			asio::coroutine CoroutineState;
			std::shared_ptr<HttpSharedState> SharedState;
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
