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
#include <windows.h>

namespace Modio
{
	namespace Detail
	{ 
		/// @docinternal
		/// @brief Used to give context when translating ms common error codes to differentiate shared codes
		enum class FilesystemErrorContext
		{
			File,
			Directory
		};

		inline Modio::ErrorCode TranslateFilesystemError(int MsError, Modio::Detail::FilesystemErrorContext Context) 
		{
			if (MsError == ERROR_SUCCESS)
			{
				return {};
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
										"Raw MS filesystem error {} received", MsError);

			switch (MsError)
			{
				case ERROR_FILE_NOT_FOUND:
				case ERROR_PATH_NOT_FOUND:
				case ERROR_BAD_PATHNAME:
				case ERROR_BAD_UNIT:
				case ERROR_NOT_READY:
				case ERROR_INVALID_DRIVE:
					if (Context == Modio::Detail::FilesystemErrorContext::File)
					{
						return Modio::make_error_code(Modio::FilesystemError::FileNotFound);
					}
					else
					{
						return Modio::make_error_code(Modio::FilesystemError::DirectoryNotFound);
					}
				case ERROR_ACCESS_DENIED:
				case ERROR_WRITE_PROTECT:
				case ERROR_LOCK_VIOLATION:
					return Modio::make_error_code(Modio::FilesystemError::NoPermission);
				case ERROR_DISK_FULL:
					return Modio::make_error_code(Modio::FilesystemError::InsufficientSpace);
				case ERROR_DIR_NOT_EMPTY:
					return Modio::make_error_code(Modio::FilesystemError::DirectoryNotEmpty);
				case ERROR_WRITE_FAULT:
					return Modio::make_error_code(Modio::FilesystemError::WriteError);
				case ERROR_READ_FAULT:
					return Modio::make_error_code(Modio::FilesystemError::ReadError);
				case ERROR_ALREADY_EXISTS:
					return Modio::make_error_code(Modio::FilesystemError::UnableToCreateFile);
				case ERROR_INVALID_PARAMETER:
					return Modio::make_error_code(Modio::GenericError::BadParameter);
			}

			return Modio::make_error_code(Modio::SystemError::UnknownSystemError);
		}
	} // namespace Detail
} // namespace Modio