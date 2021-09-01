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
		template<typename T>
		class RefSingleton
		{
		protected:
			/// <summary>
			/// </summary>
			static T Instance;

		public:
			static T& GetInstance()
			{
				return Instance;
			}
		};
				
		template<typename T>
		T RefSingleton<T>::Instance;

		template<typename T>
		class WeakSingleton
		{
		protected:
			/// <summary>
			/// We don't own the instance of ourselves here, as we don't want the singleton to extend its own
			/// lifetime. Lifetime of the shared state should be effectively managed by the HttpImplementation
			/// </summary>
			static std::weak_ptr<T> Instance;

		public:
			static std::shared_ptr<T> GetInstance()
			{
				return Instance.lock();
			}
		};

		template<typename T>
		std::weak_ptr<T> WeakSingleton<T>::Instance;
	} // namespace Detail
} // namespace Modio