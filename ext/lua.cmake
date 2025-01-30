# 
#  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

add_library(lua STATIC)

target_sources(lua PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/lua/lapi.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lauxlib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lbaselib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lcode.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lcorolib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lctype.c
	${CMAKE_CURRENT_LIST_DIR}/lua/ldblib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/ldebug.c
	${CMAKE_CURRENT_LIST_DIR}/lua/ldo.c
	${CMAKE_CURRENT_LIST_DIR}/lua/ldump.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lfunc.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lgc.c
	${CMAKE_CURRENT_LIST_DIR}/lua/linit.c
	${CMAKE_CURRENT_LIST_DIR}/lua/liolib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/llex.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lmathlib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lmem.c
	${CMAKE_CURRENT_LIST_DIR}/lua/loadlib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lobject.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lopcodes.c
	${CMAKE_CURRENT_LIST_DIR}/lua/loslib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lparser.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lstate.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lstring.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lstrlib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/ltable.c
	${CMAKE_CURRENT_LIST_DIR}/lua/ltablib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/ltm.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lundump.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lutf8lib.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lvm.c
	${CMAKE_CURRENT_LIST_DIR}/lua/lzio.c
)

target_include_directories(lua PUBLIC 
	${CMAKE_CURRENT_LIST_DIR}/lua
)
