# 
#  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#   
# 

# This function appends element to list_var if it does not already exist in list_var
function(modio_list_append_unique list_var element)
	list(FIND ${list_var} ${element} _index)
    if(_index EQUAL -1)
        list(APPEND ${list_var} ${element})
        set(${list_var} "${${list_var}}" PARENT_SCOPE)
    endif()
endfunction()
