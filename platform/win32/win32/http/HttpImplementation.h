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
#include "ModioGeneratedVariables.h"
#include "common/HttpSharedState.h"
#include "common/detail/ops/http/InitializeHttpOp.h"
#include "common/http/HttpImplementation.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/http/IHttpServiceImplementation.h"
#include "modio/http/ModioHttpParams.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <winhttp.h>
namespace Modio
{
	namespace Detail
	{
		// TODO:@modio-win32 This should be able to use the common implementation of HTTPSharedState directly perhaps
		class HttpImplementation : public Modio::Detail::HttpImplementationBase<HttpImplementation, HttpSharedStateBase>
		{
		public:
			using HttpImplementationBase::HttpImplementationBase;

			auto MakeInitializeHttpOp(std::wstring UserString, std::shared_ptr<HttpSharedStateBase> State)
			{
				return InitializeHttpOp(UserString, State);
			}
		};
	} // namespace Detail
} // namespace Modio
