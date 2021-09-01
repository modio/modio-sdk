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

#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
namespace Modio
{
	namespace Detail
	{
		class CompressionImplementation
		{
		public:
			using IOObjectImplementationType = std::shared_ptr<ArchiveFileImplementation>;

			void InitializeIOObjectImplementation(IOObjectImplementationType& Implementation)
			{
				Implementation.reset(new ArchiveFileImplementation());
			}

			void MoveIOObjectImplementation(IOObjectImplementationType& Implementation,
											IOObjectImplementationType& OtherImplementation)
			{
				Implementation = std::move(OtherImplementation);
			}

		};
	}
}
