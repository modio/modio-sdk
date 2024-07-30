#ifdef MODIO_SEPARATE_COMPILATION
	#include "jni/JavaHelpers.h"
#endif

#include "modio/core/ModioLogger.h"
#include <jni.h>
#include <string>


namespace Modio
{
	namespace Detail
	{
		std::string JavaTypeHelper::StringFromLocalRef(JNIEnv* Env, jstring JavaString) 
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

		bool JavaExceptionHelper::CheckJavaException(JNIEnv* Env)
		{
			if (!Env)
			{
				return true;
			}
			if (Env->ExceptionCheck())
			{
				std::string exceptionDetails = GetJavaExceptionDetails(Env);
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::System, "Java JNI call failed with an exception: {}", exceptionDetails);
				return true;
			}
			return false;
		}
		
		std::string JavaExceptionHelper::GetJavaExceptionDetails(JNIEnv* Env)
		{
			if (!Env || !Env->ExceptionCheck())
			{
				return "No exception or invalid JNIEnv";
			}

			jthrowable Exception = Env->ExceptionOccurred();
			Env->ExceptionClear();
    
			jclass ExceptionClass = Env->GetObjectClass(Exception);
    
			jmethodID ToStringMethod = Env->GetMethodID(ExceptionClass, "toString", "()Ljava/lang/String;");
    
			jstring StringObject = (jstring)Env->CallObjectMethod(Exception, ToStringMethod);
    
			const char* ExceptionStr = Env->GetStringUTFChars(StringObject, NULL);
    
			std::string ExceptionDetails(ExceptionStr);
    
			Env->ReleaseStringUTFChars(StringObject, ExceptionStr);
			Env->DeleteLocalRef(StringObject);
			Env->DeleteLocalRef(ExceptionClass);
			Env->DeleteLocalRef(Exception);
    
			return ExceptionDetails;
		}
	}
}