#pragma once
#pragma push_macro("FMT_HEADER_ONLY")
#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#ifdef MODIO_PLATFORM_UNREAL
	#pragma warning(push)
	#pragma warning(disable : 4583)
	#pragma warning(disable : 4582)
	#pragma warning(disable : 4265)
	#include "fmt/chrono.h"
	#include "fmt/format.h"
	#include "fmt/printf.h"
	#pragma warning(pop)
#else
	#include "fmt/chrono.h"
	#include "fmt/format.h"
	#include "fmt/printf.h"
#endif
#pragma pop_macro("FMT_HEADER_ONLY")