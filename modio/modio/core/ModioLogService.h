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
#include "modio/core/ModioServices.h"
#include "logging/LoggerImplementation.h"
#include "modio/core/ModioLogBuffer.h"
#include "modio/core/ModioLogEnum.h"
#include "modio/detail/AsioWrapper.h"

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Base class Modio logger service that support log levels and platform
		/// specific implementations
		class LogService : public asio::detail::service_base<LogService>
		{
			/// @docinternal
			/// @brief Wrapper around a Log message and the level it was logged for
			struct LogMessage
			{
				Modio::LogLevel Level;
				std::string Message;
			};

		public:
			/// @docinternal
			/// @brief Default constructor
			MODIO_IMPL explicit LogService(asio::io_context& IOService);

			using implementation_type = std::shared_ptr<Modio::Detail::LoggerImplementation>;

			MODIO_IMPL void construct(implementation_type& Implementation);
			MODIO_IMPL void destroy(implementation_type& Implementation);

			/// @brief Turn off the log service
			MODIO_IMPL void Shutdown();

			/// @brief Change the LogLevel of the log service
			MODIO_IMPL void SetLogLevel(LogLevel Level);

			/// @brief Retrieve the current LogLevel
			MODIO_IMPL LogLevel GetLogLevel() const;

			/// @brief Clear the log buffer after all stored messages are forwarded
			/// to the stored callback
			MODIO_IMPL void FlushLogBuffer();
			
			/// @docinternal
			/// @brief Log operation using a template of different possible arguments and most importantly
			///a platform specific logger object
			template<typename... ArgTypes>
			void Log(implementation_type& PlatformLoggerObject, LogLevel Level, LogCategory Category,
					 std::string Format, ArgTypes... Args)
			{
				if (Level >= CurrentLogLevel)
				{
					std::string LogOutput = PlatformLoggerObject->Log(Level, Category, Format, Args...);
					LogBuffer.push_back({Level, LogOutput});
				}
			}
			
			/// @docinternal
			/// @brief Log operation that immediately emits the formatted string
			template<typename... ArgTypes>
			void LogImmediate(implementation_type& PlatformLoggerObject, LogLevel Level, LogCategory Category,
							  std::string Format, ArgTypes... Args)
			{
				if (Level >= CurrentLogLevel)
				{
					std::string LogOutput = PlatformLoggerObject->LogImmediate(Level, Category, Format, Args...);
					LogBuffer.push_back({Level, LogOutput});
				}
			}

			/// @docinternal
			/// @brief Configure the callback receiver of string elements
			static MODIO_IMPL void SetLogCallback(std::function<void(Modio::LogLevel, const std::string&)> LogCallback);
			
			/// @docinternal
			/// @brief Static function to permit easy setting of global logging level
			/// @param Level The minimum severity level to display
			static MODIO_IMPL void SetGlobalLogLevel(Modio::LogLevel Level);

		private:
			// Not stored in global state, as it's created during startup, and we want to be able to capture logs from
			// the startup
			static MODIO_IMPL std::function<void(Modio::LogLevel, const std::string&)> UserCallbackFunction;

			LogLevel CurrentLogLevel = LogLevel::Warning;
			asio::strand<asio::io_context::executor_type> LogStrand;
			std::vector<LogMessage> LogBuffer {};
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioLogService.ipp"
#endif
