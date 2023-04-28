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
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"

#include <asio/yield.hpp>
class DeleteFileOp
{
public:
	DeleteFileOp(Modio::filesystem::path FilePath, std::weak_ptr<Modio::Detail::FileSharedState> SharedState)
		: FilePath(FilePath), SharedState(SharedState) {};
	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		if (std::shared_ptr<Modio::Detail::FileSharedState> PinnedState = SharedState.lock())
		{
			if (PinnedState->bCancelRequested)
			{
				Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
				return;
			}
		}
		else
		{
			Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
			return;
		}
		reenter(CoroState)
		{
			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Begin delete of {}",
										FilePath.string());
			yield asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));
			Modio::ErrorCode RemoveStatus;
			Modio::filesystem::remove(FilePath, RemoveStatus);
			Self.complete(RemoveStatus);
		}
	}

private:
	asio::coroutine CoroState;
	Modio::filesystem::path FilePath;
	std::weak_ptr<Modio::Detail::FileSharedState> SharedState;
};
#include <asio/unyield.hpp>