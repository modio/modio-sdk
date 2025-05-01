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
		JavaClassWrapperModio::JavaClassWrapperModio(jobject Activity, bool bUseExternalStorageForMods)
			: JavaClassWrapper("com/modio/modiosdk/Modio", "(Landroid/app/Activity;Z)V", Activity,
							   bUseExternalStorageForMods)
		{
			JNIEnv* Env = Modio::Detail::AndroidContextService::Get().GetJavaEnv();
			if (Env == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to get JNI environment");
				return;
			}

			if (Class == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find class: com/modio/modiosdk/Modio");
				return;
			}
			
			GetCertificatePathMethodId = Env->GetMethodID(Class, "getCertificatePath", "()Ljava/lang/String;");
			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || GetCertificatePathMethodId == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find method getCertificatePath in class Modio");
				return;
			}
			GetInternalStoragePathMethodId = Env->GetMethodID(Class, "getInternalStorageDirectory", "()Ljava/lang/String;");
			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || GetInternalStoragePathMethodId == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find method getInternalStorageDirectory in class Modio");
				return;
			}
			GetExternalStoragePathMethodId = Env->GetMethodID(Class, "getExternalStorageDirectory", "()Ljava/lang/String;");
			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || GetExternalStoragePathMethodId == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find method getExternalStorageDirectory in class Modio");
				return;
			}
		} 

		std::string JavaClassWrapperModio::GetCertificatePath()
		{
			return CallStringMethod(GetCertificatePathMethodId);
		}

		std::string JavaClassWrapperModio::GetInternalStoragePath()
		{
			return CallStringMethod(GetInternalStoragePathMethodId);
		}

		std::string JavaClassWrapperModio::GetExternalStoragePath()
		{
			return CallStringMethod(GetExternalStoragePathMethodId);
		}
	}
}
