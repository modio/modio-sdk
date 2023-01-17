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
#include "modio/detail/ModioDefines.h"

#include "modio/detail/OptionalWrapper.h"
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <system_error>

namespace asio
{
	class mutable_buffer;
	class const_buffer;
} // namespace asio

namespace Modio
{
	/// @docpublic
	/// @brief Trivial wrapper around link:https://en.cppreference.com/w/cpp/error/error_code[std::error_code].
	/// Implemented as a class instead of a type alias to allow it to be forward-declared in wrappers, eg the UE4
	/// plugin.
	class ErrorCode : public std::error_code
	{
	public:
		/// @docpublic
		/// @brief Error code constructor using std::error_code
		ErrorCode(std::error_code ec) : error_code(ec) {};
		
		/// @docpublic
		/// @brief Error code constructor using an empty value
		ErrorCode() : error_code() {};
		using error_code::error_code;
	};

	/// @docpublic
	/// @brief nullable wrapper around object of type T. Used by async functions that return values - empty on function
	/// error/failure
	/// @tparam T
	template<typename T>
	using Optional = tl::optional<T>;

	using MutableBufferView = asio::mutable_buffer;
	using ConstBufferView = asio::const_buffer;

	template<typename Object>
	using StableStorage = std::shared_ptr<Object>;

	/// @docpublic
	/// @brief Convenience method to make a shared pointer
	template<typename Object, class... _Types>
	StableStorage<Object> MakeStable(_Types&&... _Args)
	{
		return std::make_shared<Object>(std::forward<_Types>(_Args)...);
	}

	using Timestamp = std::uint64_t;
	using GalleryIndex = std::size_t;

	/// @brief Type alias to a local profile identifier used per user for initialization
	using UserHandleType = std::string;

} // namespace Modio
