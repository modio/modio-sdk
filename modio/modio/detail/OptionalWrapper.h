#pragma once
#ifdef MODIO_PLATFORM_UNREAL
	#pragma warning(push)
	#pragma warning(disable : 4583)
	#pragma warning(disable : 4582)
	#include "tl/optional.hpp"
	#pragma warning(pop)
#else
	#include "tl/optional.hpp"
#endif