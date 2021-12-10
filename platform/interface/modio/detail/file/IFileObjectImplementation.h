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
#include "modio/core/ModioStdTypes.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/AsioWrapper.h"

namespace Modio
{
	namespace Detail
	{
		enum class SeekDirection
		{
			Absolute,
			Forward,
			Backward
		};
		/// @brief Interface class defining operations that a platform file object needs to support
		class IFileObjectImplementation
		{
		public:
			virtual ~IFileObjectImplementation() {};

			virtual void SetFileStrand(asio::strand<asio::io_context::executor_type>& FileStrand) = 0;

			virtual asio::strand<asio::io_context::executor_type>& GetFileStrand() = 0;

			virtual Modio::ErrorCode CreateFile(filesystem::path FilePath) = 0;

			virtual Modio::ErrorCode OpenFile(filesystem::path FilePath, bool bOverwrite = false) = 0;

			/// @brief OS-specific file size calculation
			/// @return Size of the underlying file
			/// @todo {core} Should this return std::uintmax_t instead?
			virtual std::size_t GetSize() = 0;
			/// @brief Retrieves file path
			/// @return Path to the underlying file
			virtual Modio::filesystem::path GetPath() = 0;
			/// @brief Renames a file to a new name
			/// @param NewPath The new name for the file
			/// @return true on success
			virtual Modio::ErrorCode Rename(Modio::filesystem::path NewPath) = 0;
			/// @brief Sets the file length to the desired value
			/// @param Offset new end length for the file
			/// @todo {core} should this truncate the seek pointer?
			virtual void Truncate(Modio::FileOffset Offset) = 0;
			/// @brief Sets the offset for future streamed reads or writes to the file
			/// @param Offset The position to read or write from
			virtual void Seek(Modio::FileOffset Offset, Modio::Detail::SeekDirection Direction) = 0;

			virtual void Destroy() = 0;
		};
	} // namespace Detail
} // namespace Modio