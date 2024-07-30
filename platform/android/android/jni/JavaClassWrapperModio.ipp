#ifdef MODIO_SEPARATE_COMPILATION
	#include "jni/JavaClassWrapperModio.h"
#endif
#include "jni/AndroidContextService.h"
#include "modio/core/ModioLogger.h"
#include "jni/JavaClassWrapper.h"
#include "jni/JavaHelpers.h"

namespace Modio
{
	namespace Detail
	{
		JavaClassWrapperModio::JavaClassWrapperModio(jobject Activity)
			:
		JavaClassWrapper("com/modio/modiosdk/Modio", "(Landroid/app/Activity;)V", Activity)
		{
			JNIEnv* Env = Modio::Detail::AndroidContextService::Get().GetJavaEnv();
			
			GetCertificatePathMethodId = Env->GetMethodID(Class, "getCertificatePath", "()Ljava/lang/String;");
			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || GetCertificatePathMethodId == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find method getCertificatePath in class Modio");
				return;
			}
			GetStoragePathMethodId = Env->GetMethodID(Class, "getStorageDirectory", "()Ljava/lang/String;");
			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || GetStoragePathMethodId == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find method getStorageDirectory in class Modio");
				return;
			}
		} 

		std::string JavaClassWrapperModio::GetCertificatePath()
		{
			return CallStringMethod(GetCertificatePathMethodId);
		}

		std::string JavaClassWrapperModio::GetStoragePath()
		{
			return CallStringMethod(GetStoragePathMethodId);
		}
	}
}
