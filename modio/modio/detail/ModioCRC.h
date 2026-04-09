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

#include "modio/core/ModioSDKForwardDecls.h"

namespace Modio
{
	namespace Detail
	{
		class Buffer;

		/// @brief Simple byte-at-a-time CRC implementation. Could be sped up by slicing by 4+, but this would require
		/// disabling strict aliasing
		/// @param Data Buffer containing the data to CRC. Passed by reference because we don't actually want to consume
		/// the data in the buffer, just read it
        /// @param PreviousCRC32 Previous CRC value allowing the chaining of multiple calls to this function
        /// @param UntilByte If only a section of the Buffer Data is required, marks the finish line of bytes to read
		/// @return the calculated checksum value
		uint32_t CRC32(Modio::Detail::Buffer& Data, uint32_t PreviousCRC32 = 0,
					   Modio::Optional<std::size_t> UntilByte = Modio::Optional<size_t> {});

	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioCRC.ipp"
#endif
