#pragma once

#include <jni.h>
#include <string>
#include "modio/core/ModioSplitCompilation.h"

namespace Modio
{
	namespace Detail
	{
		// Helper class that automatically calls DeleteLocalRef on the passed-in Java object when goes out of scope
		template<typename T>
		class ScopedJavaObject
		{
		public:
			ScopedJavaObject(JNIEnv* InEnv, const T& InObjRef) : Env(InEnv), ObjRef(InObjRef) {}

			ScopedJavaObject(ScopedJavaObject&& Other) : Env(Other.Env), ObjRef(Other.ObjRef)
			{
				Other.Env = nullptr;
				Other.ObjRef = nullptr;
			}

			ScopedJavaObject(const ScopedJavaObject& Other) = delete;
			ScopedJavaObject& operator=(const ScopedJavaObject& Other) = delete;

			~ScopedJavaObject()
			{
				if (*this)
				{
					Env->DeleteLocalRef(ObjRef);
				}
			}

			// Returns the underlying JNI pointer
			T operator*() const
			{
				return ObjRef;
			}

			operator bool() const
			{
				if (!Env || !ObjRef || Env->IsSameObject(ObjRef, NULL))
				{
					return false;
				}

				return true;
			}

		private:
			JNIEnv* Env = nullptr;
			T ObjRef = nullptr;
		};

		template<typename T>
		ScopedJavaObject<T> NewScopedJavaObject(JNIEnv* InEnv, const T& InObjRef)
		{
			return ScopedJavaObject<T>(InEnv, InObjRef);
		}


		/// @docinternal
		///	@brief Helper class for converting between Java and C++ types, including cleanup
		class JavaTypeHelper
		{
		public:
			/// @docinternal
			/// @brief Converts a Java string to a C++ string
			/// @param Env The JNI environment
			/// @param JavaString The Java string to convert
			/// @return The converted C++ string
			MODIO_IMPL static std::string StringFromLocalRef(JNIEnv* Env, jstring JavaString);
		};

		/// @docinternal
		///	@brief Class for handling Java exceptions
		class JavaExceptionHelper
		{
		public:
			/// @docinternal
			/// @brief Checks if a Java exception has occurred and logs it if it has
			/// @return true if an exception has occurred
			MODIO_IMPL static bool CheckJavaException(JNIEnv* Env);

			/// @docinternal
			/// @brief Gets the details of the current Java exception
			/// @return A string containing the exception details
			MODIO_IMPL static std::string GetJavaExceptionDetails(JNIEnv* Env);
		};
	}
}


#ifndef MODIO_SEPARATE_COMPILATION
	#include "JavaHelpers.ipp"
#endif