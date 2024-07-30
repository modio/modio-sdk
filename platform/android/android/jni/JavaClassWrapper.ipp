#ifdef MODIO_SEPARATE_COMPILATION
	#include "jni/JavaClassWrapper.h"
#endif
#include "jni/JavaHelpers.h"
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

			if (ClassLoader != NULL)
			{
				jmethodID FindClassMethod = Modio::Detail::AndroidContextService::Get().GetFindClassMethod();

				jstring ClassNameObj = Env->NewStringUTF("com/modio/modiosdk/Modio");
				LocalClass = static_cast<jclass>(Env->CallObjectMethod(ClassLoader, FindClassMethod, ClassNameObj));
				Env->DeleteLocalRef(ClassNameObj);
				if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || LocalClass == NULL)
				{
					Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core,
												"Failed to find class: " + ClassName);
					return;
				}

				Class = (jclass)Env->NewGlobalRef(LocalClass);
				Env->DeleteLocalRef(LocalClass);
			}
			else
			{
				// Get the class
				LocalClass = Env->FindClass(ClassName.c_str());
				if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || LocalClass == NULL)
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
			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || LocalConstructor == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find constructor for class: " + ClassName);
				return;
			}
			
			// Create the object
			va_list Args;
			va_start(Args, ConstructorSignature);
			auto LocalObject = Env->NewObjectV(Class, LocalConstructor, Args);
			va_end(Args);

			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || LocalObject == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to create object for class: " + ClassName);
				return;
			}

			Object = Env->NewGlobalRef(LocalObject);
		}

		JavaClassWrapper::~JavaClassWrapper()
		{
			auto Env = Modio::Detail::AndroidContextService::Get().GetJavaEnv();

			Env->DeleteGlobalRef(Object);
			Env->DeleteGlobalRef(Class);
		}

		bool JavaClassWrapper::CallBooleanMethod(jmethodID Method, ...)
		{
			if (Method == NULL || Object == NULL)
			{
				return false;
			}

			JNIEnv* Env = Modio::Detail::AndroidContextService::Get().GetJavaEnv();

			va_list Args;
			va_start(Args, Method);
			jboolean Return = Env->CallBooleanMethodV(Object, Method, Args);
			va_end(Args);

			return (bool) Return;
		}

		std::string JavaClassWrapper::CallStringMethod(jmethodID Method, ...)
		{
			if (Method == NULL || Object == NULL)
			{
				return NULL;
			}

			JNIEnv* Env = Modio::Detail::AndroidContextService::Get().GetJavaEnv();

			va_list Args;
			va_start(Args, Method);
			jobject Return = Env->CallObjectMethodV(Object, Method, Args);
			va_end(Args);

			std::string Result = Modio::Detail::JavaTypeHelper::StringFromLocalRef(Env, (jstring) Return);

			return Result;
		}

		void JavaClassWrapper::CallVoidMethod(jmethodID Method, ...)
		{
			if (Method == NULL || Object == NULL)
			{
				return;
			}

			JNIEnv* Env = Modio::Detail::AndroidContextService::Get().GetJavaEnv();

			va_list Args;
			va_start(Args, Method);
			Env->CallVoidMethodV(Object, Method, Args);
			va_end(Args);
		}
	}
}
