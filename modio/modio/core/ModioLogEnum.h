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
#include "modio/core/ModioCoreTypes.h"

namespace Modio
{
	/// @brief Converts a Modio::LogLevel enum to string for use in log output
	/// @param Level the enum level
	/// @return Null-terminated string containing friendly name
	constexpr const char* LogLevelToString(LogLevel Level)
	{
		switch (Level)
		{
			case LogLevel::Trace:
				return "Trace";
			case LogLevel::Info:
				return "Info";
			case LogLevel::Warning:
				return "Warning";
			case LogLevel::Error:
				return "Error";
			default:
				return "UNKNOWN";
		}
	}

	/// @brief Converts a Modio::LogCategory enum to string for use in log output
	/// @param Category the enum category
	/// @return Null-terminated string containing friendly name
	constexpr const char* LogCategoryToString(LogCategory Category)
	{
		switch (Category)
		{
			case LogCategory::Core:
				return "Core";
			case LogCategory::File:
				return "File";
			case LogCategory::Http:
				return "Http";
			case LogCategory::Compression:
				return "Compression";
			case LogCategory::User:
				return "User";
			case LogCategory::ModManagement:
				return "ModManagement";
			default:
				return "UNKNKOWN";
		}
	}
} // namespace Modio
