/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

// PImpl definitions for AppleHttpSession and AppleHttpRequest.
// Only include from .mm (Obj-C++) translation units.

#pragma once

#ifndef __OBJC__
	#error "AppleHttp_Internal.h must only be included from Obj-C++ (.mm) translation units."
#endif

#include "ios/AppleHttpRequest.h"
#include "ios/AppleHttpSession.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioErrorCode.h"
#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#import <Foundation/Foundation.h>

@class ModioHttpDelegate;

namespace Modio
{
	namespace Detail
	{
		namespace Apple
		{
			// Maps NSURLErrorDomain codes to SDK error codes.
			// Unknown codes map to HttpError::RequestError.
			Modio::ErrorCode TranslateUrlErrorCode(long NSURLErrorCode);

			struct HttpSession::Impl
			{
				NSURLSession* Session = nil;
				ModioHttpDelegate* Delegate = nil;
				NSOperationQueue* DelegateQueue = nil;
				std::string UserAgent;
				std::atomic<bool> Closing {false};

				// Signaled from didBecomeInvalidWithError. Close() waits on this
				// to ensure no delegate callback outlives the session.
				dispatch_semaphore_t InvalidationSemaphore = dispatch_semaphore_create(0);

				// Task-to-request map. Holds weak_ptrs so delegate callbacks can
				// safely lock a shared_ptr even if the owning HttpRequest is
				// being destroyed concurrently.
				std::mutex MapMutex;
				std::unordered_map<std::uint64_t, std::weak_ptr<HttpRequest::Impl>> TaskToRequest;

				void RegisterRequest(std::uint64_t TaskIdentifier, std::weak_ptr<HttpRequest::Impl> Request);
				void UnregisterTask(std::uint64_t TaskIdentifier);
				std::shared_ptr<HttpRequest::Impl> LookupRequest(std::uint64_t TaskIdentifier);
			};

			struct HttpRequest::Impl
			{
				explicit Impl(std::weak_ptr<HttpSession::Impl> Owner) : OwnerSession(std::move(Owner)) {}

				// Weak so the request can safely outlive its session.
				std::weak_ptr<HttpSession::Impl> OwnerSession;

				NSMutableURLRequest* Request = nil;
				NSURLSessionTask* Task = nil;
				// Output stream for streamed uploads; paired with the request's HTTPBodyStream.
				NSOutputStream* UploadWriteStream = nil;

				// Polled lock-free by the SDK thread. Writers must hold StateMutex
				// to prevent races and ensure companion fields are visible before
				// the state becomes observable.
				std::atomic<HttpRequest::State> CurrentState {HttpRequest::State::Pending};

				mutable std::mutex StateMutex;

				// Populated before transitioning to HeadersReceived.
				std::uint32_t ResponseCode = 0;
				std::vector<std::pair<std::string, std::string>> ResponseHeaders;

				// Body chunks buffered by the delegate, drained by the SDK thread.
				std::deque<Modio::Detail::Buffer> ReceivedBuffers;
				std::size_t ReceivedBytesPending = 0;

				// Set on transition to Failed.
				Modio::ErrorCode FinalError {};

				// Attempts a state transition while StateMutex is held.
				// Refuses to move past terminal states. Returns true on success.
				bool TryAdvanceStateLocked(HttpRequest::State Target);

				// Delegate-queue callbacks.
				void OnDidReceiveResponse(std::uint32_t StatusCode, std::vector<std::pair<std::string, std::string>> Headers);
				void OnDidReceiveData(const std::uint8_t* Bytes, std::size_t Length);
				void OnDidComplete(Modio::ErrorCode Error);
			};
		} // namespace Apple
	} // namespace Detail
} // namespace Modio