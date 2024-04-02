#ifdef MODIO_SEPARATE_COMPILATION
	#include "jni/JavaClassWrapper.h"
#endif
#include "jni/AndroidContextService.h"
#include "modio/core/ModioLogger.h"

namespace Modio
{
	namespace Detail
	{
		JavaClassWrapper::JavaClassWrapper(std::string ClassName, const char* ConstructorSignature, ...)
		{
			AndroidContextService& AndroidService = Modio::Detail::AndroidContextService::Get();

			auto Env = AndroidService.GetJavaEnv();

			jobject ClassLoader = Modio::Detail::AndroidContextService::Get().GetClassLoader();

			jclass LocalClass;

			if (ClassLoader != nullptr)
			{
				jmethodID FindClassMethod = Modio::Detail::AndroidContextService::Get().GetFindClassMethod();

				jstring ClassNameObj = Env->NewStringUTF("com/modio/modiosdk/Modio");
				LocalClass = static_cast<jclass>(Env->CallObjectMethod(ClassLoader, FindClassMethod, ClassNameObj));
				Env->DeleteLocalRef(ClassNameObj);

				Class = (jclass) Env->NewGlobalRef(LocalClass);
				Env->DeleteLocalRef(LocalClass);
			}
			else
			{
				// Get the class
				LocalClass = Env->FindClass(ClassName.c_str());
				if (LocalClass == NULL)
				{
					Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core,
												"Failed to find class: " + ClassName);
					return;
				}
				Class = (jclass) Env->NewGlobalRef(LocalClass);
				Env->DeleteLocalRef(LocalClass);

			}

			// Get the constructor
			jmethodID LocalConstructor = Env->GetMethodID(Class, "<init>", ConstructorSignature);
			if (LocalConstructor == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find constructor for class: " + ClassName);
				return;
			}
			
			// Create the object
			va_list Args;
			va_start(Args, ConstructorSignature);
			auto LocalObject = Env->NewObjectV(Class, LocalConstructor, Args);
			va_end(Args);

			Object = Env->NewGlobalRef(LocalObject);
		}

		JavaClassWrapper::~JavaClassWrapper()
		{
			auto Env = Modio::Detail::AndroidContextService::Get().GetJavaEnv();

			Env->DeleteGlobalRef(Object);
			Env->DeleteGlobalRef(Class);
		}
	}
}
