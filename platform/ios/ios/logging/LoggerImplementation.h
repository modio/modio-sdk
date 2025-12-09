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
#include "modio/core/ModioLogEnum.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"

namespace Modio
{
	static auto StartTime = std::chrono::system_clock::now();
	namespace Detail
	{
		/// @docinternal
		/// @brief Helper implementation class to organize logging operations and formatting
		class LoggerImplementation
		{
			ModioAsio::strand<ModioAsio::io_context::executor_type>& LogStrand;

		public:
			/// @docinternal
			/// @brief LoggerImplementation constructor
			LoggerImplementation(ModioAsio::strand<ModioAsio::io_context::executor_type>& Strand) : LogStrand(Strand) {}
			
			/// @docinternal
			/// @brief Receive an formatted string and transform it to another string that fits 
			/// the LoggerImplementation structure, given LogLevel and Logcategory
			/// @param LogLevel the severity of the log, from a Trace to Error
			/// @param LogCategory it relates to the subsystem emitting this log
			/// @param Format The basic string to log
			/// @param Args Extra arguments used in the Format string
			template<typename... ArgTypes>
			std::string Log(LogLevel Level, LogCategory Category, std::string Format, ArgTypes... Args)
			{
				constexpr const char* LogFormatString = "[{:%H:%M:%S}:{}][{}][{}] {}\r\n";
				
				std::string LogUserString = fmt::format(Format, Args...);
				auto Now = std::chrono::system_clock::now();
				// @todo: OutputDebugStringA might crash if the string is too big, so it would be nice to split the
				// string up or make some solution that is reliable when using big strings
				auto FormattedOutput =
				fmt::format(LogFormatString, Now,
							std::chrono::duration_cast<std::chrono::milliseconds>(Now.time_since_epoch()) % 1000,
							LogLevelToString(Level), LogCategoryToString(Category), LogUserString);
				ModioAsio::post(LogStrand, [FormattedOutput]() { fmt::print(fmt::runtime(FormattedOutput)); });
				return FormattedOutput;
			}

			/// @docinternal
			/// @brief Immediately uses the current context to log output. Not intended for general usage
			template<typename... ArgTypes>
			std::string LogImmediate(LogLevel Level, LogCategory Category, std::string Format, ArgTypes... Args)
			{
				constexpr const char* LogFormatString = "[{:%H:%M:%S}:{}][{}][{}] {}\r\n";
				
				std::string LogUserString = fmt::format(fmt::runtime(Format), Args...);
				auto Now = std::chrono::system_clock::now();
				// @todo: OutputDebugStringA might crash if the string is too big, so it would be nice to split the
				// string up or make some solution that is reliable when using big strings
				auto FormattedOutput =
				fmt::format(fmt::runtime(LogFormatString), Now,
							std::chrono::duration_cast<std::chrono::milliseconds>(Now.time_since_epoch()) % 1000,
							LogLevelToString(Level), LogCategoryToString(Category), LogUserString);
				fmt::print("{}", FormattedOutput.c_str());
				return FormattedOutput;
			}
		};
	} // namespace Detail
} // namespace Modio
