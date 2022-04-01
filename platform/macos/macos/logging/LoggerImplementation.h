/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once
#include "modio/core/ModioLogEnum.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"

namespace Modio
{
	static auto StartTime = std::chrono::system_clock::now();
	namespace Detail
	{
		class LoggerImplementation
		{
			asio::strand<asio::io_context::executor_type>& LogStrand;

		public:
			LoggerImplementation(asio::strand<asio::io_context::executor_type>& Strand) : LogStrand(Strand) {}

			template<typename... ArgTypes>
			std::string Log(LogLevel Level, LogCategory Category, std::string Format, ArgTypes... Args)
			{
				std::string LogFormatString = "[{:%H:%M:%S}:{}][{}][{}] {}\r\n";

				std::string LogUserString = fmt::format(Format, Args...);
				auto Now = std::chrono::system_clock::now();
				// @todo: OutputDebugStringA might crash if the string is too big, so it would be nice to split the
				// string up or make some solution that is reliable when using big strings
				auto FormattedOutput =
					fmt::format(LogFormatString, Now,
								std::chrono::duration_cast<std::chrono::milliseconds>(Now.time_since_epoch()) % 1000,
								LogLevelToString(Level), LogCategoryToString(Category), LogUserString);
				asio::post(LogStrand, [FormattedOutput]() { fmt::print(FormattedOutput.c_str()); });
				return FormattedOutput;
			}

			/// <summary>
			/// Immediately uses the current context to log output. Not intended for general usage
			/// </summary>
			template<typename... ArgTypes>
			std::string LogImmediate(LogLevel Level, LogCategory Category, std::string Format, ArgTypes... Args)
			{
				std::string LogFormatString = "[{:%H:%M:%S}:{}][{}][{}] {}\r\n";

				std::string LogUserString = fmt::format(Format, Args...);
				auto Now = std::chrono::system_clock::now();
				// @todo: OutputDebugStringA might crash if the string is too big, so it would be nice to split the
				// string up or make some solution that is reliable when using big strings
				auto FormattedOutput =
					fmt::format(LogFormatString, Now,
								std::chrono::duration_cast<std::chrono::milliseconds>(Now.time_since_epoch()) % 1000,
								LogLevelToString(Level), LogCategoryToString(Category), LogUserString);
				fmt::print("{}", FormattedOutput.c_str());
				return FormattedOutput;
			}
		};
	} // namespace Detail
} // namespace Modio
