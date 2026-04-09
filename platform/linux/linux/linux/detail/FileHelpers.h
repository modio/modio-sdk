/*
 *  Copyright (C) 2021-2026 mod.io Pty Ltd. <https://mod.io>
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
#include <cerrno>

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Used to give context when translating linux error codes to differentiate shared codes
		enum class FilesystemErrorContext
		{
			File,
			Directory
		};

		inline Modio::ErrorCode TranslateFilesystemError(int LinuxError, Modio::Detail::FilesystemErrorContext Context)
		{
			if (LinuxError == 0)
			{
				return {};
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
										"Raw Linux filesystem error {} received", LinuxError);

			switch (LinuxError)
			{
				case ENOENT:
					if (Context == Modio::Detail::FilesystemErrorContext::File)
					{
						return Modio::make_error_code(Modio::FilesystemError::FileNotFound);
					}
					else
					{
						return Modio::make_error_code(Modio::FilesystemError::DirectoryNotFound);
					}
				case EACCES:
				case EROFS:
				case EPERM:
					return Modio::make_error_code(Modio::FilesystemError::NoPermission);
				case ENOSPC:
					return Modio::make_error_code(Modio::FilesystemError::InsufficientSpace);
				case ENOTEMPTY:
					return Modio::make_error_code(Modio::FilesystemError::DirectoryNotEmpty);
				case EINVAL:
					return Modio::make_error_code(Modio::GenericError::BadParameter);
			}

			return Modio::make_error_code(Modio::SystemError::UnknownSystemError);
		}
	} // namespace Detail
} // namespace Modio