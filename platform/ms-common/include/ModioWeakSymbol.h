#pragma once

extern "C"
{
	inline void modio_profile_stub(...) {};
	__declspec(selectany) void (*modio_profile_stub_ptr)(...) = modio_profile_stub;
}

#define MODIO_WEAK(Name)                                                  \
	__pragma(comment(linker, "/alternatename:" #Name "=modio_profile_stub"));
