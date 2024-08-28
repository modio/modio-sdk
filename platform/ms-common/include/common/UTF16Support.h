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

#include "modio/detail/ModioCompilerMacros.h"
MODIO_DISABLE_WARNING_PUSH
MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_CONVERSION
MODIO_DISABLE_WARNING_SPECTRE_MITIGATION
MODIO_DISABLE_WARNING_STRUCTURE_PADDING
MODIO_DISABLE_WARNING_UNSAFE_BUFFER_USAGE
MODIO_DISABLE_WARNING_UNUSED_FUNCTION
#include "utf8.h"
MODIO_DISABLE_WARNING_POP
#include <string>

inline const std::wstring UTF8ToWideChar(std::string UTF8String)
{
	std::wstring Result;
	utf8::utf8to16(UTF8String.begin(), UTF8String.end(), back_inserter(Result));
	return Result;
}

inline const std::string WideCharToUTF8(std::wstring WideString)
{
	std::string Result;
	utf8::utf16to8(WideString.begin(), WideString.end(), back_inserter(Result));
	return Result;
}