

#ifdef MODIO_SEPARATE_COMPILATION
	#include "ModioAndroid.h"
#endif

#include "jni/AndroidContextService.h"
#include "modio/core/ModioServices.h"

namespace Modio
{
	void InitializeAndroidJNI(JavaVM* InJavaVM, jobject ClassLoader)
	{
		Modio::Detail::AndroidContextService::InitializeJNI(InJavaVM, ClassLoader);
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
