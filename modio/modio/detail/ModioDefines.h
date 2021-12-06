/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *   
 */

// Provides library-wide defines controlling configuration of third-party libraries, etc. 

// We have custom DEBUG_RELEASE flags, as in unreal, we can't define DEBUG/_DEBUG as
// unreal has modified the allocator in debug, so we can't allocate all our objects with
// the standard allocator
#ifndef MODIO_DEBUG
	#if defined(DEBUG) || defined(_DEBUG)
		#define MODIO_DEBUG 1
	#else
		#define MODIO_DEBUG 0
	#endif
#endif

#ifndef MODIO_RELEASE
	#if defined(NDEBUG)
		#define MODIO_RELEASE 1
	#else
		#define MODIO_RELEASE 0
	#endif
#endif

// If we should track how many of each operation there are alive at all time
// useful if we want to find leaking operations and zombie operations
#ifndef MODIO_TRACK_OPS
	#define MODIO_TRACK_OPS 0
#endif

// BEGIN ASIO DEFINES
#ifndef ASIO_STANDALONE
	#define MODIO_DEFINED_ASIO_STANDALONE
	#define ASIO_STANDALONE
#endif

#ifndef ASIO_HAS_STD_ADDRESSOF
	#define MODIO_DEFINED_ASIO_HAS_STD_ADDRESSOF
	#define ASIO_HAS_STD_ADDRESSOF
#endif

/*#ifndef ASIO_NO_EXCEPTIONS
	#define MODIO_DEFINED_ASIO_NO_EXCEPTIONS
	#define ASIO_NO_EXCEPTIONS
#endif*/

#ifndef ASIO_HAS_STD_ARRAY
	#define MODIO_DEFINED_ASIO_HAS_STD_ARRAY
	#define ASIO_HAS_STD_ARRAY
#endif

#ifndef ASIO_HAS_CSTDINT
	#define MODIO_DEFINED_ASIO_HAS_CSTDINT
	#define ASIO_HAS_CSTDINT
#endif

#ifndef ASIO_HAS_STD_SHARED_PTR
	#define MODIO_DEFINED_ASIO_HAS_STD_SHARED_PTR
	#define ASIO_HAS_STD_SHARED_PTR
#endif

#ifndef ASIO_HAS_STD_TYPE_TRAITS
	#define MODIO_DEFINED_ASIO_HAS_STD_TYPE_TRAITS
	#define ASIO_HAS_STD_TYPE_TRAITS
#endif

#ifndef ASIO_HAS_VARIADIC_TEMPLATES
	#define MODIO_DEFINED_ASIO_HAS_VARIADIC_TEMPLATES
	#define ASIO_HAS_VARIADIC_TEMPLATES
#endif

#ifndef ASIO_HAS_STD_FUNCTION
	#define MODIO_DEFINED_ASIO_HAS_STD_FUNCTION
	#define ASIO_HAS_STD_FUNCTION
#endif

#ifndef ASIO_HAS_STD_CHRONO
	#define MODIO_DEFINED_ASIO_HAS_STD_CHRONO
	#define ASIO_HAS_STD_CHRONO
#endif

#ifndef ASIO_HAS_MOVE
	#define MODIO_DEFINED_ASIO_HAS_MOVE
	#define ASIO_HAS_MOVE
#endif

#ifndef ASIO_NO_DEFAULT_LINKED_LIBS
	#define MODIO_DEFINED_ASIO_NO_DEFAULT_LINKED_LIBS
	#define ASIO_NO_DEFAULT_LINKED_LIBS
#endif

#ifndef ASIO_DISABLE_IOCP
	#define MODIO_DEFINED_ASIO_DISABLE_IOCP
	#define ASIO_DISABLE_IOCP
#endif

#ifndef ASIO_NO_DEPRECATED
	#define MODIO_DEFINED_ASIO_NO_DEPRECATED
	#define ASIO_NO_DEPRECATED
#endif

#ifndef ASIO_NO_DEPRECATED
	#define MODIO_DEFINED_ASIO_NO_DEPRECATED
	#define ASIO_NO_DEPRECATED
#endif

#ifndef BOOST_ALL_NO_LIB
	#define MODIO_DEFINED_BOOST_ALL_NO_LIB
	#define BOOST_ALL_NO_LIB
#endif
// END ASIO DEFINES

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#define MODIO_DEFINED_WIN32_LEAN_AND_MEAN
#endif

#ifndef FMT_HEADER_ONLY
	#define MODIO_DEFINED_FMT_HEADER_ONLY
	#define FMT_HEADER_ONLY
#endif

#ifndef NOMINMAX
	#define MODIO_DEFINED_NOMINMAX
	#define NOMINMAX
#endif

#ifndef D_UNICODE
	#define MODIO_DEFINED_D_UNICODE
	#define D_UNICODE
#endif

#ifndef UNICODE
	#define MODIO_DEFINED_UNICODE
	#define UNICODE
#endif

