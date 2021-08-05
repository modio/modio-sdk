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