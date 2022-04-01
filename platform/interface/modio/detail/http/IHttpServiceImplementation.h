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

namespace Modio
{
	namespace Detail
	{
		class IHttpServiceImplementation
		{
		public:
			virtual ~IHttpServiceImplementation() {};
			virtual void Shutdown() = 0;

			// The implementation of HTTP request on a new platform requires
			// at minimum the following HTTP operations to perform:
			// - Shared state: It shared data between operations
			// - Initialize request: It perform the necessary alloc/init transactions
			// - Send request: Once it is ready, it sends the HTTP request over the wire
			// - Write to request: When data is split in multiple packages, this operation
			//   sends those bytes over a open socket/stream.
			// - Read Header response: It parses the HTTP standard headers and the
			//   response code, either 200, 400, 500, etc
			// - Read Body Response: It parses the rest of the HTTP request according to
			//   some standarized data types: json, URL encoded, binary, etc.
			//
			// For reference, the "mscommon" and "macos" implementations perform these
			// operations, the first using WinHTTP and the second relying on CFNetwork
		};
	} // namespace Detail
} // namespace Modio
