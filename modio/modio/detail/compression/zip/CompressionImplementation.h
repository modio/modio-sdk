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
