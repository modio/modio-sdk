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

#include "utf8.h"
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