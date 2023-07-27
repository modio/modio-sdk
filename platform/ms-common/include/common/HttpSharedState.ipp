/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "common/HttpSharedState.h"
#endif

#include "common/http/HttpRequestImplementation.h"
#include "modio/detail/ModioProfiling.h"

HttpSharedStateBase::HttpSharedStateBase(HINTERNET SessionHandle) : CurrentSession(SessionHandle) {}
HttpSharedStateBase::HttpSharedStateBase(HttpSharedStateBase&& Other)
{
	*this = std::move(Other);
}
HttpSharedStateBase& HttpSharedStateBase::operator=(HttpSharedStateBase&& Other)
{
	// We're going to steal all the implementation of the other object so ensure that another thread isnt touching it
	auto Lock = Other.GetInsertLock();
	CallbackStatus = std::move(Other.CallbackStatus);
	ExtendedStatus = std::move(Other.ExtendedStatus);
	ConnectionHandles = std::move(Other.ConnectionHandles);
	CurrentServiceState = std::move(Other.CurrentServiceState);
	CurrentSession = std::move(Other.CurrentSession);
	return *this;
}

void HttpSharedStateBase::InitializeRequest(std::shared_ptr<HttpRequestImplementation> Request, Modio::ErrorCode& ec)
{
	MODIO_PROFILE_SCOPE(InitializeRequest);
	Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http, "Initializing {0} request for {1} {2}",
								Request->GetParameters().GetVerb(), Request->GetParameters().GetServerAddress(),
								Request->GetParameters().GetFormattedResourcePath());
	//"https://api.test.mod.io"
	// WinHttpConnect always blocks so no need to spin this off as an async operation
	HINTERNET ConnectionHandle = nullptr;

	{
		MODIO_PROFILE_SCOPE(WinHttpConnect);

		ConnectionHandle =
			WinHttpConnect(CurrentSession, UTF8ToWideChar(Request->GetParameters().GetServerAddress()).c_str(),
						   INTERNET_DEFAULT_HTTPS_PORT, 0);
	}

	if (ConnectionHandle == nullptr)
	{
		auto ConnectionStatus = GetLastError();
		if (ConnectionStatus == ERROR_WINHTTP_OPERATION_CANCELLED || ConnectionStatus == ERROR_WINHTTP_SHUTDOWN)
		{
			ec = Modio::make_error_code(Modio::GenericError::OperationCanceled);
			return;
		}
		else
		{
			ec = Modio::make_error_code(Modio::HttpError::CannotOpenConnection);
			return;
		}
	}

	// TODO: @ModioGDK specify accept types properly
	// constexpr auto AcceptJsonType = L"application/json";
	HINTERNET RequestHandle = nullptr;

	{
		MODIO_PROFILE_SCOPE(WinHttpOpenRequest);
		RequestHandle =
			WinHttpOpenRequest(ConnectionHandle, UTF8ToWideChar(Request->GetParameters().GetVerb()).c_str(),
							   UTF8ToWideChar(Request->GetParameters().GetFormattedResourcePath()).c_str(), nullptr,
							   WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	}

	if (RequestHandle == nullptr)
	{
		ec = Modio::make_error_code(Modio::HttpError::CannotOpenConnection);
		return;
	}
	else
	{
		{
			// We're performing an insert so need to take the exclusive lock on the collection
			auto Lock = GetInsertLock();
			// ConnectionHandles[RequestHandle] = ConnectionHandle;
			CallbackStatus[RequestHandle] = WinHTTPCallbackStatus::Waiting;
			ExtendedStatus[RequestHandle] = std::make_pair(0, 0);
		}
		Request->ConnectionHandle = ConnectionHandle;
		Request->RequestHandle = RequestHandle;

		ec = {};
		return;
	}
}

Modio::Optional<std::atomic<WinHTTPCallbackStatus>&> HttpSharedStateBase::FindCallbackStatusForRequest(
	HINTERNET RequestHandle)
{
	auto Lock = GetReadOrModifyLock();
	auto CallbackStatusIterator = CallbackStatus.find(RequestHandle);
	if (CallbackStatusIterator != CallbackStatus.end())
	{
		return (*CallbackStatusIterator).second;
	}
	else
	{
		return {};
	}
}

Modio::Optional<std::pair<std::uintptr_t, std::intmax_t>&> HttpSharedStateBase::FindExtendedStatusForRequest(
	HINTERNET RequestHandle)
{
	auto Lock = GetReadOrModifyLock();
	auto ExtendedStatusIterator = ExtendedStatus.find(RequestHandle);
	if (ExtendedStatusIterator != ExtendedStatus.end())
	{
		return (*ExtendedStatusIterator).second;
	}
	else
	{
		return {};
	}
}

void HttpSharedStateBase::SetHandleStatus(HINTERNET Handle, WinHTTPCallbackStatus Status, void* ExtendedStatusData,
										  unsigned long ExtendedStatusLength)
{
	{
		// This performs an insertion so needs the writer/insertion lock
		auto Lock = GetInsertLock();
		ExtendedStatus[Handle] = std::make_pair((std::uintptr_t) ExtendedStatusData, ExtendedStatusLength);
	}
	{
		auto Lock = GetReadOrModifyLock();
		if (auto MutableCallbackStatus = FindCallbackStatusForRequest(Handle))
		{
			MutableCallbackStatus->exchange(Status);
		}
	}
}

WinHTTPCallbackStatus HttpSharedStateBase::PeekHandleStatus(HINTERNET Handle)
{
	if (auto MutableCallbackStatus = FindCallbackStatusForRequest(Handle))
	{
		return MutableCallbackStatus->load();
	}
	return WinHTTPCallbackStatus::RequestError;
}

WinHTTPCallbackStatus HttpSharedStateBase::FetchAndClearHandleStatus(HINTERNET Handle)
{
	auto Lock = GetReadOrModifyLock();
	if (auto MutableCallbackStatus = FindCallbackStatusForRequest(Handle))
	{
		return MutableCallbackStatus->exchange(WinHTTPCallbackStatus::Waiting);
	}
	return WinHTTPCallbackStatus::RequestError;
}

void HttpSharedStateBase::EraseCabllbackStatus(HINTERNET Handle)
{
	CallbackStatus.erase(Handle);
	ExtendedStatus.erase(Handle);
}

std::pair<std::uintptr_t, std::uintmax_t> HttpSharedStateBase::GetExtendedStatus(HINTERNET Handle)
{
	auto Lock = GetReadOrModifyLock();
	if (auto MutableExtendedStatus = FindExtendedStatusForRequest(Handle))
	{
		auto StatusValues = *MutableExtendedStatus;
		*MutableExtendedStatus = std::make_pair(0, 0);
		return StatusValues;
	}
	return std::make_pair(0, 0);
}

bool HttpSharedStateBase::IsClosing()
{
	return CurrentServiceState == HttpServiceState::Closing;
}

void HttpSharedStateBase::Close()
{
	CurrentServiceState = HttpServiceState::Closing;
	WinHttpSetOption(CurrentSession, WINHTTP_OPTION_CONTEXT_VALUE, nullptr, sizeof(uintptr_t));
	for (const auto Handle : ConnectionHandles)
	{
		WinHttpSetStatusCallback(Handle.first, nullptr, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
		if (Handle.second != nullptr)
		{
			WinHttpCloseHandle(Handle.second);
		}
	}
	// Ensure that we update all of our callbacks and close all handles when we shut down
	WinHttpSetStatusCallback(CurrentSession, nullptr, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
	if (CurrentSession != nullptr)
	{
		WinHttpCloseHandle(CurrentSession);
	}
}
