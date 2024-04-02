#ifdef MODIO_SEPARATE_COMPILATION
	#include "jni/JavaHelpers.h"
#endif

#include <jni.h>
#include <string>


namespace Modio
{
	namespace Detail
	{
		std::string JavaHelpers::StringFromLocalRef(JNIEnv* Env, jstring JavaString) 
		{
			if (!Env || !JavaString || Env->IsSameObject(JavaString, NULL))
			{
				return {};
			}

			const char* convertedValue = (Env)->GetStringUTFChars(JavaString, 0);
			std::string str = convertedValue;

			Env->ReleaseStringUTFChars(JavaString, convertedValue);

			if (Env && JavaString)
			{
				Env->DeleteLocalRef(JavaString);
			}

			return str;
		}
	}
}