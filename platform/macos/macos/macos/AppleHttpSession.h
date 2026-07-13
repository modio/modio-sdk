/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioErrorCode.h"
#include <memory>
#include <string>

namespace Modio
{
	namespace Detail
	{
		namespace Apple
		{
			class HttpRequest;

			// C++ interface. Owns the NSURLSession, its delegate, and the delegate queue.
			// One instance per SDK lifetime.
			class HttpSession
			{
			public:
				HttpSession();
				~HttpSession();

				HttpSession(const HttpSession&) = delete;
				HttpSession& operator=(const HttpSession&) = delete;

				Modio::ErrorCode Initialize(const std::string& UserAgent);

				// Creates a new Pending request bound to this session.
				std::unique_ptr<HttpRequest> CreateRequest();

				// Cancels all in-flight tasks
				void Close();
				bool IsClosing() const;

				// Public so the Obj-C delegate in AppleHttpSession.mm can
				// name the type. Defined in AppleHttp_Internal.h.
				struct Impl;

			private:
				friend class HttpRequest;
				// shared_ptr so HttpRequest can hold a weak_ptr, preventing
				// dangling access if a request outlives the session.
				std::shared_ptr<Impl> PImpl;
			};
		} // namespace Apple
	} // namespace Detail
} // namespace Modio