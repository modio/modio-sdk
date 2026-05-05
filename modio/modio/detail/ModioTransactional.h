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
	template<typename TargetType>
	class Transaction
	{
		std::weak_ptr<TargetType> TargetObject {};
		bool bCommitted = false;
		bool bMovedFrom = false;

	public:
		Transaction() : TargetObject(), bCommitted(false) {}
		Transaction(std::weak_ptr<TargetType> TargetObject) : TargetObject(TargetObject)
		{
			if (std::shared_ptr<TargetType> ResolvedTarget = TargetObject.lock())
			{
				// Unqualified function name forces ADL to find the hidden friend declared in TargetType
				BeginTransactionImpl(*ResolvedTarget);
			}
		}
		Transaction(Transaction&& Other)
		{
			TargetObject = std::move(Other.TargetObject);
			bCommitted = std::move(Other.bCommitted);
			Other.bMovedFrom = true;
		}
		Transaction& operator=(Transaction&& Other)
		{
			TargetObject = std::move(Other.TargetObject);
			bCommitted = std::move(Other.bCommitted);
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

	namespace Detail
	{
		template<typename BaseClass>
		class Transactional
		{
			friend Modio::Transaction<BaseClass> BeginTransaction(std::shared_ptr<BaseClass> Target)
			{
				return Modio::Transaction<BaseClass>(Target);
			}
		};
	} // namespace Detail
} // namespace Modio