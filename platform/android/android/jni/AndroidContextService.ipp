/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "jni/AndroidContextService.h"
#endif

#include "jni/JavaClassWrapperModio.h"
#include <inttypes.h>
#include <jni.h>
#include "jni/JavaHelpers.h"
#include "modio/core/ModioLogger.h"

#define JNI_CURRENT_VERSION JNI_VERSION_1_6

namespace Modio
{
	namespace Detail
	{
		AndroidContextService& AndroidContextService::Get()
		{
			static AndroidContextService Instance;
			return Instance;
		}

		void AndroidContextService::InternalInitializeJNI(JavaVM* InJavaVM, jobject InClassLoader)
		{
			JVM = InJavaVM;
			ClassLoader = InClassLoader;

			JNIEnv* Env = GetJavaEnv();
			if (Env == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to get JNIEnv");
				return;
			}

			jclass environmentClass = Env->FindClass("android/os/Environment");
			if (environmentClass == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find Environment class");
				return;
			}

			auto EnvClassObj = Modio::Detail::NewScopedJavaObject(Env, environmentClass);
			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || !EnvClassObj)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to create Environment class object");
				return;
			}

			// Find the ClassLoader.findClass method
			jclass classLoaderClass = Env->FindClass("java/lang/ClassLoader");
			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || classLoaderClass == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find ClassLoader class");
				return;
			}
			FindClassMethod = Env->GetMethodID(classLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
			if (Modio::Detail::JavaExceptionHelper::CheckJavaException(Env) || FindClassMethod == NULL)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Failed to find ClassLoader.findClass method");
				return;
			}
		}

		void AndroidContextService::InitializeJNI(JavaVM* InJavaVM, jobject InClassLoader)
		{
			auto& AndroidContextService = Modio::Detail::AndroidContextService::Get();

			AndroidContextService.InternalInitializeJNI(InJavaVM, InClassLoader);
		}

		JNIEnv* AndroidContextService::GetJavaEnv()
		{
			if (JVM == NULL)
			{
				return nullptr;
			}
		    JNIEnv* Env = nullptr;
			jint GetEnvResult = JVM->GetEnv((void**) &Env, JNI_CURRENT_VERSION);

			if (GetEnvResult == JNI_EDETACHED)
			{
				jint AttachResult = JVM->AttachCurrentThread(&Env, NULL);

				if (AttachResult == JNI_ERR)
				{
					return nullptr;
				}
			} else if (GetEnvResult != JNI_OK) 
			{
				return nullptr;
			}

			return Env;
		}

		void AndroidContextService::SetGlobalActivity(jobject InActivityObject)
		{
			ActivityObject = InActivityObject;
		};

		void AndroidContextService::InitializeAndroid()
		{
			JavaClassModio = new JavaClassWrapperModio(ActivityObject);
		}

	} // namespace Detail
} // namespace Modio
