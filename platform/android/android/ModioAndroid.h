#pragma once
#include "modio/detail/ModioLibraryConfigurationHelpers.h"

#include <jni.h>
#include <string>

namespace Modio
{
	/// @docpublic
	///	@brief Initializes the JNI environment for the Plugin to be able to set up all of its bindings
	///	@param InJavaVM Pointer to the JavaVM from the host application
	///	@param InClassLoader ClassLoader from the host application
	///	@param bUseExternalStorageForMods Flag to specify whether to use external storage for storing downloaded mods (default is true)
	MODIOSDK_API void InitializeAndroidJNI(JavaVM* InJavaVM, jobject InClassLoader,
										   bool bUseExternalStorageForMods = true);

	/// @docpublic
	///	@brief Sets the global activity object for the Plugin to be able to call Java methods
	///	@param ActivityObject Main ActivityObject from the host Java application
	MODIOSDK_API void SetGlobalActivity(jobject ActivityObject);

	/// @docpublic
	///	@brief Initialize the Modio SDK for Android. Called after InitializeAndroidJNI and SetGlobalActivity
	MODIOSDK_API void InitializeAndroid();
}

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioAndroid.ipp"
#endif