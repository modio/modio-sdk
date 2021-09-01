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
#include "common/HttpSharedState.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/detail/FmtWrapper.h"
#include <memory>


/// @brief Win32 http shared state is just a thin wrapper around the common functionality
class HttpSharedState : public std::enable_shared_from_this<HttpSharedState>, public HttpSharedStateBase
{
public:
	using HttpSharedStateBase::HttpSharedStateBase;
};
