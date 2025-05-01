/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "logging/LoggerImplementation.h"
#include "modio/core/ModioLogBuffer.h"
#include "modio/core/ModioLogEnum.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"

#include <jni.h>

namespace Modio
{
	namespace Detail
	{
		class JavaClassWrapperModio;

		/// @docinternal
		/// @brief Service for holding all the context for the Android platform, including JNI interop
		class AndroidContextService
		{
			
		public:
			/// @docinternal
			/// @brief Sets the reference to the Global activity and Modio object from the host Java application
			MODIO_IMPL void SetGlobalActivity(jobject InActivityObject);

			/// @docinternal
			/// @brief Initialize the JNI and bindings internally
			static MODIO_IMPL void InitializeJNI(JavaVM* InJavaVM, jobject InClassLoader,
												 bool bInUseExternalStorageForMods);

			/// @docinternal
			///	@brief Get the JNI Environment, attached to the current thread as appropriate
			MODIO_IMPL JNIEnv* GetJavaEnv();

			/// @docinternal
			///	@brief Helper for getting the native class wrapper around the Java Modio class
			MODIO_IMPL JavaClassWrapperModio* GetJavaClassModio() const
			{
				return JavaClassModio;
			}

			/// @docinternal
			///	@brief Helper for getting the passed in ClassLoader for loading non-core Java classes
			MODIO_IMPL jobject GetClassLoader()
			{
				return ClassLoader;
			}

			/// @docinternal
			///	@brief Helpre for getting the FindClass method from the ClassLoader for loading non-core Java classes
			MODIO_IMPL jmethodID GetFindClassMethod()
			{
				return FindClassMethod;
			}

			/// @docinternal
			///	@brief Initialize the modio Android SDK. Called after InitializeJNI and SetGlobalActivity
			MODIO_IMPL void InitializeAndroid();

			MODIO_IMPL static AndroidContextService& Get();

		private:
			MODIO_IMPL void InternalInitializeJNI(JavaVM* InJavaVM, jobject InClassLoader,
												  bool bUseExternalStorageForMods);

			jobject ActivityObject = NULL;

			JavaVM* JVM = NULL;

			jobject ClassLoader = NULL;
			jmethodID FindClassMethod = NULL;

			JavaClassWrapperModio* JavaClassModio = NULL;

			bool bUseExternalStorageForMods = true;
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "AndroidContextService.ipp"
#endif