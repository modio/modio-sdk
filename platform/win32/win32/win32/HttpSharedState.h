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
