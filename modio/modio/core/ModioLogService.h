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
#include "modio/core/ModioServices.h"
#include "logging/LoggerImplementation.h"
#include "modio/core/ModioLogBuffer.h"
#include "modio/core/ModioLogEnum.h"
#include "modio/detail/AsioWrapper.h"

namespace Modio
{
	namespace Detail
	{
		class LogService : public asio::detail::service_base<LogService>
		{
			struct LogMessage
			{
				Modio::LogLevel Level;
				std::string Message;
			};

		public:
			MODIO_IMPL explicit LogService(asio::io_context& IOService);

			using implementation_type = std::shared_ptr<Modio::Detail::LoggerImplementation>;

			MODIO_IMPL void construct(implementation_type& Implementation);
			MODIO_IMPL void destroy(implementation_type& Implementation);

			MODIO_IMPL void Shutdown();

			MODIO_IMPL void SetLogLevel(LogLevel Level);

			MODIO_IMPL LogLevel GetLogLevel() const;

			MODIO_IMPL void FlushLogBuffer();

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
			static MODIO_IMPL void SetLogCallback(std::function<void(Modio::LogLevel, const std::string&)> LogCallback);
			
			/// @brief Static function to permit easy setting of global logging level
			/// @param Level The minimum severity level to display
			static MODIO_IMPL void SetGlobalLogLevel(Modio::LogLevel Level);

		private:
			// Not stored in global state, as it's created during startup, and we want to be able to capture logs from
			// the startup
			static MODIO_IMPL std::function<void(Modio::LogLevel, const std::string&)> UserCallbackFunction;

			LogLevel CurrentLogLevel;
			asio::strand<asio::io_context::executor_type> LogStrand;
			std::vector<LogMessage> LogBuffer;
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioLogService.ipp"
#endif