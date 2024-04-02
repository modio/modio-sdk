#pragma once

#include "modio/core/ModioSplitCompilation.h"
#include <jni.h>
#include <string>

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Base class that wraps any native Java classes that need to be instantiated
		///	and called for any Java-based Modio functionality
		class JavaClassWrapper
		{
		public:
			MODIO_IMPL JavaClassWrapper(std::string ClassName, const char* ConstructorSignature, ...);
			MODIO_IMPL ~JavaClassWrapper();

		protected:
			jobject Object;
			jclass Class;
		};
	}; // namespace Detail

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "JavaClassWrapper.ipp"
#endif