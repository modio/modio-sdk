/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *   
 */

// Restores the earlier state of some defines that is changed in ModioDefines.h
// so that your library isn't affected by them
#pragma once

#ifdef MODIO_DEFINED_FMT_HEADER_ONLY
	#undef FMT_HEADER_ONLY
#endif

#ifdef MODIO_DEFINED_BOOST_BEAST_DECL
	#undef MODIO_DEFINED_BOOST_BEAST_DECL
#endif

#ifdef MODIO_DEFINED_NOMINMAX
	#undef NOMINMAX
#endif

#ifdef MODIO_DEFINED_D_UNICODE
	#undef D_UNICODE
#endif

#ifdef MODIO_DEFINED_UNICODE
	#undef UNICODE
#endif

#ifdef MODIO_DEFINED_WIN32_LEAN_AND_MEAN
	#undef WIN32_LEAN_AND_MEAN
#endif

// BEGIN ASIO DEFINES
#ifndef MODIO_DEFINED_ASIO_STANDALONE
	#undef MODIO_DEFINED_ASIO_STANDALONE
#endif

#ifdef MODIO_DEFINED_ASIO_HAS_STD_ADDRESSOF
	#undef ASIO_HAS_STD_ADDRESSOF
#endif

/*#ifdef MODIO_DEFINED_ASIO_NO_EXCEPTIONS
	#undef ASIO_NO_EXCEPTIONS
#endif*/

#ifdef MODIO_DEFINED_ASIO_HAS_STD_ARRAY
	#undef ASIO_HAS_STD_ARRAY
#endif

#ifdef MODIO_DEFINED_ASIO_HAS_CSTDINT
	#undef ASIO_HAS_CSTDINT
#endif

#ifdef MODIO_DEFINED_ASIO_HAS_STD_SHARED_PTR
	#undef ASIO_HAS_STD_SHARED_PTR
#endif

#ifdef MODIO_DEFINED_ASIO_HAS_STD_TYPE_TRAITS
	#undef ASIO_HAS_STD_TYPE_TRAITS
#endif

#ifdef MODIO_DEFINED_ASIO_HAS_VARIADIC_TEMPLATES
	#undef ASIO_HAS_VARIADIC_TEMPLATES
#endif

#ifdef MODIO_DEFINED_ASIO_HAS_STD_FUNCTION
	#undef ASIO_HAS_STD_FUNCTION
#endif

#ifdef MODIO_DEFINED_ASIO_HAS_STD_CHRONO
	#undef ASIO_HAS_STD_CHRONO
#endif

#ifdef MODIO_DEFINED_ASIO_HAS_MOVE
	#undef ASIO_HAS_MOVE
#endif

#ifdef MODIO_DEFINED_ASIO_NO_DEFAULT_LINKED_LIBS
	#undef ASIO_NO_DEFAULT_LINKED_LIBS
#endif

#ifdef MODIO_DEFINED_ASIO_DISABLE_IOCP
	#undef ASIO_DISABLE_IOCP
#endif

#ifdef MODIO_DEFINED_ASIO_NO_DEPRECATED
	#undef ASIO_NO_DEPRECATED
#endif

#ifdef MODIO_DEFINED_ASIO_NO_DEPRECATED
	#undef ASIO_NO_DEPRECATED
#endif

#ifdef MODIO_DEFINED_BOOST_ALL_NO_LIB
	#undef BOOST_ALL_NO_LIB
#endif

// END ASIO DEFINES