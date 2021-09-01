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
#include "common/UTF16Support.h"
#include "common/http/HttpRequestImplementation.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/detail/FmtWrapper.h"
#include <atomic>
#include <memory>
#include <queue>
#include <unordered_map>
#include <utility>

// TODO: @ModioGDK probably move this back up to the parent service
enum class HttpServiceState
{
	Running,
	Closing
};

enum class WinHTTPCallbackStatus : unsigned long
{
	Waiting = 0,
	HeadersAvailable = WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE,
	DataAvailable = WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE,
	ReadComplete = WINHTTP_CALLBACK_STATUS_READ_COMPLETE,
	SendRequestComplete = WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE,
	WriteComplete = WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE,
	RequestError = WINHTTP_CALLBACK_STATUS_REQUEST_ERROR
};

class HttpSharedStateBase : public std::enable_shared_from_this<HttpSharedStateBase>
{
	/// <summary>
	/// Maps a WinHTTP Request handle to its most recent status from the WINHTTP_STATUS_CALLBACK
	/// This status value is in turn responsible for driving the internal state machine of a WinHTTP HTTPRequest
	/// operation
	/// </summary>
	std::unordered_map<HINTERNET, std::atomic<WinHTTPCallbackStatus>> CallbackStatus;

	/// <summary>
	/// Maps a WinHTTP Request handle to a buffer of memory containing the extended status information for the last
	/// callback invocation
	/// </summary>
	std::unordered_map<HINTERNET, std::pair<std::uintptr_t, std::intmax_t>> ExtendedStatus;

	/// <summary>
	/// Maps a WinHTTP Request handle to its corresponding Connection handle
	/// According to GDK documentation, neither of these should be reused, and there's a 1:1
	/// relationship between Request handles and connection Handles
	/// So this allows us to find the connection handle for a given request
	/// and close both of them at the same time
	/// </summary>
	std::unordered_map<HINTERNET, HINTERNET> ConnectionHandles;

protected:
	/// <summary>
	/// Explicit enum allowing us to signal to all running HTTP operations that they should cancel
	/// </summary>
	HttpServiceState CurrentServiceState = HttpServiceState::Running;

public:
	// TODO: @ModioGDK ENSURE SESSION HANDLE IS ONLY CANCELLED AFTER ALL Connection and request handles are closed
	HINTERNET CurrentSession;
	MODIO_IMPL HttpSharedStateBase(HINTERNET SessionHandle);
	HttpSharedStateBase& operator=(HttpSharedStateBase&& Other) = default;

	MODIO_IMPL void InitializeRequest(std::shared_ptr<HttpRequestImplementation> Request, Modio::ErrorCode& ec);

	MODIO_IMPL void SetHandleStatus(HINTERNET Handle, WinHTTPCallbackStatus Status, void* ExtendedStatusData,
									unsigned long ExtendedStatusLength);

	MODIO_IMPL WinHTTPCallbackStatus PeekHandleStatus(HINTERNET Handle);

	MODIO_IMPL WinHTTPCallbackStatus FetchAndClearHandleStatus(HINTERNET Handle);
	MODIO_IMPL std::pair<std::uintptr_t, std::uintmax_t> GetExtendedStatus(HINTERNET Handle);
	MODIO_IMPL bool IsClosing();
	MODIO_IMPL void Close();
};

#ifndef MODIO_SEPARATE_COMPILATION
	#include "HttpSharedState.ipp"
#endif