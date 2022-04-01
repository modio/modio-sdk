/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
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
	DeleteFileOp(Modio::filesystem::path FilePath) : FilePath(FilePath) {};
	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Begin delete of {}",
									FilePath.string());
		Modio::ErrorCode RemoveStatus;
		Modio::filesystem::remove(FilePath, RemoveStatus);
		Self.complete(RemoveStatus);
	}

private:
	Modio::filesystem::path FilePath;
};
#include <asio/unyield.hpp>