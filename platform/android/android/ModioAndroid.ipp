

#ifdef MODIO_SEPARATE_COMPILATION
	#include "ModioAndroid.h"
#endif

#include "jni/AndroidContextService.h"
#include "modio/core/ModioServices.h"

namespace Modio
{
	void InitializeAndroidJNI(JavaVM* InJavaVM, jobject ClassLoader, bool bUseExternalStorageForMods)
	{
		Modio::Detail::AndroidContextService::InitializeJNI(InJavaVM, ClassLoader, bUseExternalStorageForMods);
	}

	void SetGlobalActivity(jobject ActivityObject) 
	{
		Modio::Detail::AndroidContextService::Get().SetGlobalActivity(ActivityObject);
	}

	void InitializeAndroid()
	{
		Modio::Detail::AndroidContextService::Get().InitializeAndroid();
	}
}
