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
#include <memory>

namespace Modio
{
	namespace Detail
	{
		template<typename TargetType>
		class Transaction
		{
			bool bCommitted = false;
			std::weak_ptr<TargetType> TargetObject;
			bool bMovedFrom = false;

		public:
			Transaction() : bCommitted(false), TargetObject() {};
			Transaction(std::weak_ptr<TargetType> TargetObject) : TargetObject(TargetObject)
			{
				if (std::shared_ptr<TargetType> ResolvedTarget = TargetObject.lock())
				{
					// Unqualified function name forces ADL to find the hidden friend declared in TargetType
					BeginTransactionImpl(*ResolvedTarget);
				}
			};
			Transaction(Transaction&& Other)
			{
				bCommitted = std::move(Other.bCommitted);
				TargetObject = std::move(Other.TargetObject);
				Other.bMovedFrom = true;
			}
			Transaction& operator=(Transaction&& Other)
			{
				bCommitted = std::move(Other.bCommitted);
				TargetObject = std::move(Other.TargetObject);
				Other.bMovedFrom = true;
				return *this;
			}
			~Transaction()
			{
				if (!bMovedFrom && !bCommitted)
				{
					if (std::shared_ptr<TargetType> ResolvedTarget = TargetObject.lock())
					{
						RollbackTransactionImpl(*ResolvedTarget);
					}
				}
			}
			void Commit()
			{
				if (TargetObject.lock())
				{
					bCommitted = true;
				}
			}
		};

		template<typename BaseClass>
		class Transactional
		{
			friend Transaction<BaseClass> BeginTransaction(std::shared_ptr<BaseClass> Target)
			{
				return Transaction<BaseClass>(Target);
			}
		};
	} // namespace Detail
} // namespace Modio