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

#include <utility>
#include <vector>

namespace Modio
{
	namespace Detail
	{
		inline std::string ToString(const std::string& Value)
		{
			return Value;
		}

		inline std::string ToString(std::int64_t Value)
		{
			return std::to_string(Value);
		}

		/// @brief Helper class which stores key-value pairs for HTTP request bodies
		class RequestBodyKVPContainer
		{
			using InternalContainerClass = std::vector<std::pair<std::string, std::string>>;
			InternalContainerClass InternalContainer;

		public:
			template<typename T>
			void Add(const std::string& Key, const T& Value)
			{
				InternalContainer.push_back({Key, ToString(Value)});
			}

			template<typename T>
			void Add(const std::string& Key, const Modio::Optional<T>& Value)
			{
				if (Value)
				{
					InternalContainer.push_back({Key, ToString(Value.value())});
				}
			}

			using iterator = typename InternalContainerClass::iterator;
			using const_iterator = typename InternalContainerClass::const_iterator;
			iterator begin()
			{
				return InternalContainer.begin();
			}

			const_iterator begin() const
			{
				return InternalContainer.begin();
			}

			iterator end()
			{
				return InternalContainer.end();
			}

			const_iterator end() const
			{
				return InternalContainer.end();
			}
		};
	} // namespace Detail
} // namespace Modio
