#pragma once
#include "ModioGeneratedVariables.h"
namespace Modio
{
	namespace Detail
	{
		struct BaseOperationCommonImpl
		{
			static MODIO_IMPL bool RequiresShutdown;
		};

		template<typename Base>
		struct BaseOperation : public BaseOperationCommonImpl
		{
#if MODIO_TRACK_OPS
			inline static int32_t Count = 0;
			bool bMovedFrom = false;
			BaseOperation();
			BaseOperation(BaseOperation&& Other);
			~BaseOperation();
#endif
		};
	} // namespace Detail
} // namespace Modio
#ifndef MODIO_SEPARATE_COMPILATION
#include "modio/impl/detail/ModioObjectTrack.ipp"
#endif