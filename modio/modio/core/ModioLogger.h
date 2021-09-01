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
#include "modio/core/ModioLogService.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"

namespace Modio
{
	namespace Detail
	{
		/// @brief Generic threadsafe logging object. Can either construct and retain an instance, or log via a
		/// temporary
		class Logger : public asio::basic_io_object<Modio::Detail::LogService>
		{
		protected:
			template<typename... ArgTypes>
			void LogImmediate(LogLevel Level, LogCategory Category, std::string Format, ArgTypes... Args)
			{
				get_service().LogImmediate(get_implementation(), Level, Category, Format, Args...);
			}

		public:
			/// @brief Explicit constructor for a Logger that posts messages via an explicit io_context
			/// @param Context the io_context to use
			explicit Logger(asio::io_context& Context) : asio::basic_io_object<Modio::Detail::LogService>(Context) {}

			/// @brief Explicit convenience constructor for a Logger that posts messages via the global SDK io_context
			explicit Logger()
				: asio::basic_io_object<Modio::Detail::LogService>(Modio::Detail::Services::GetGlobalContext())
			{}

			/// @brief Print a message to the platform output device
			/// @tparam ...ArgTypes Types of arguments to format
			/// @param Level The verbosity level of the message
			/// @param Category The category for the message - corresponds to which internal service is originating the
			/// message
			/// @param Format Format string for the message. Uses fmtlib syntax for substitutions
			/// @param ...Args Additional arguments to insert into the format string
			template<typename... ArgTypes>
			void Log(LogLevel Level, LogCategory Category, std::string Format, ArgTypes... Args)
			{
				get_service().LogImmediate(get_implementation(), Level, Category, Format, Args...);
			}
		};
		/// @brief Simple class to log entry and exit for a particular scope
		class ScopedLogger : Logger
		{
			std::string ScopeName;
			LogCategory Category;

		public:
			/// @brief Constructs a scoped logger object. Will print a message containing ScopeName when created, and
			/// when the scope exits
			/// @param Category The log category to use
			/// @param ScopeName The name of the scope to print
			explicit ScopedLogger(LogCategory Category, std::string ScopeName)
				: ScopeName(ScopeName),
				  Category(Category)
			{
				LogImmediate(LogLevel::Info, Category, "Entered {}", ScopeName);
			}
			~ScopedLogger()
			{
				LogImmediate(LogLevel::Info, Category, "Left {}", ScopeName);
			}
		};
	} // namespace Detail
} // namespace Modio