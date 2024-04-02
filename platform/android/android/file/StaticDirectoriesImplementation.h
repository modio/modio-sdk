
#pragma once

#include <string>
#include "jni/AndroidContextService.h"
#include "jni/JavaClassWrapperModio.h"

namespace Modio
{
	namespace Detail
	{
		static bool GetDefaultCommonDataPath(Modio::filesystem::path& CommonDataPath)
		{
			CommonDataPath = Modio::Detail::AndroidContextService::Get()
					.GetJavaClassModio()->GetStoragePath();
			
			return true;
		}
	} // namespace Detail
} // namespace Modio
