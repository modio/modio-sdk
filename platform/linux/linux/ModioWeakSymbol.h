#pragma once

extern "C"
{
#ifdef __clang__
	inline void modio_profile_stub(...) {};
#else
	static inline void modio_profile_stub(...) {};
#endif
}

#define MODIO_WEAK(Name) __attribute__((weak, alias("modio_profile_stub")))
