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
		class JavaHelpers
		{
		public:
			MODIO_IMPL static std::string StringFromLocalRef(JNIEnv* Env, jstring JavaString);
		};
	}
}


#ifndef MODIO_SEPARATE_COMPILATION
	#include "JavaHelpers.ipp"
#endif