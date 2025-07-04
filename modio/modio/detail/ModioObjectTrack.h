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
#include "modio/core/ModioSplitCompilation.h"
#include <unordered_map>

namespace Modio
{
	namespace Detail
	{
		class OperationTracker
		{
			const char* OpName = nullptr;
			bool bMovedFrom = false;
			void* OpPtr = nullptr;

			static std::unordered_map<void*, const char*>& OperationMap()
			{
				static std::unordered_map<void*, const char*> OperationMapInst;
				return OperationMapInst;
			}

		public:
			/// @docinternal
			/// @brief Default destructor
			MODIO_IMPL ~OperationTracker();

			/// @docinternal
			/// @brief Explicit constructor
			explicit OperationTracker(const char* OpName, void* OpPtr) : OpName(OpName), OpPtr(OpPtr)
			{
				OperationMap()[OpPtr] = OpName;
			}

			template<typename OpClass>
			explicit OperationTracker(const char* OpName, OpClass* OpPtr)
				: OperationTracker(OpName, static_cast<void*>(OpPtr)) {}

			/// @docinternal
			/// @brief ScopedProfileEvent move constructor
			OperationTracker(OperationTracker&& Other)
			{
				*this = std::move(Other);
				Other.bMovedFrom = true;
			}

			/// @docinternal
			/// @brief ScopedProfileEvent assignment constructor
			MODIO_IMPL OperationTracker& operator=(OperationTracker&& Other);

			/// @docnone
			OperationTracker(const OperationTracker& Other) = delete;

			/// @docnone
			OperationTracker& operator=(const OperationTracker& Other) = delete;
		};

#define MODIO_TRACKED_OP(Name) \
	Modio::Detail::OperationTracker TrackedOp_##Name = Modio::Detail::OperationTracker(#Name, this)

		struct BaseOperationCommonImpl
		{
			static MODIO_IMPL bool RequiresShutdown;
		};

		template<typename Base>
		struct BaseOperation : public BaseOperationCommonImpl
		{
#if defined(MODIO_TRACK_OPS) && (MODIO_TRACK_OPS != 0)
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