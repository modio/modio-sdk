
#pragma once

#include <string>
#include "jni/AndroidContextService.h"
#include "jni/JavaClassWrapperModio.h"

namespace Modio
{
	namespace Detail
	{
		static bool GetDefaultRootDataPath(Modio::filesystem::path& RootDataPath)
		{
			RootDataPath = Modio::Detail::AndroidContextService::Get().GetJavaClassModio()->GetExternalStoragePath();
			return true;
		}
		static bool GetDefaultCommonDataPath(Modio::filesystem::path& CommonDataPath)
		{
			CommonDataPath = Modio::Detail::AndroidContextService::Get()
					.GetJavaClassModio()->GetInternalStoragePath();
			
			return true;
		}
	} // namespace Detail
} // namespace Modio
