#pragma once
// These are necessary because we're pulling in windows.h via ghc fileystem
#include "ModioGeneratedVariables.h"
#include "modio/detail/ModioDefines.h"

#include "modio/detail/FilesystemWrapper.h"
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
	template<typename Rep, typename Period = std::ratio<1>>
	using Duration = std::chrono::duration<Rep, Period>;

	using ErrorCode = std::error_code;

	/// @docpublic
	/// @brief nullable wrapper around object of type T. Used by async functions that return values - empty on function
	/// error/failure
	/// @tparam T
	template<typename T>
	using Optional = tl::optional<T>;

	// Backport of std::filesystem to support C++14/C++11
	namespace filesystem = ghc::filesystem;

	using MutableBufferView = asio::mutable_buffer;
	using ConstBufferView = asio::const_buffer;

	template<typename Object>
	using StableStorage = std::shared_ptr<Object>;

	template<typename Object, class... _Types>
	StableStorage<Object> MakeStable(_Types&&... _Args)
	{
		return std::make_shared<Object>(std::forward<_Types>(_Args)...);
	}

	using Timestamp = std::uint64_t;
	using GalleryIndex = int;

	/// @brief Type alias for User IDs, used for initialization
	using UserHandleType = std::string;

} // namespace Modio
