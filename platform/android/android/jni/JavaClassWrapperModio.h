#pragma once

#include "JavaClassWrapper.h"
#include "modio/core/ModioSplitCompilation.h"
#include <jni.h>
#include <string>

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		///	@brief WRapper around the internal core Modio Java class
		class JavaClassWrapperModio : JavaClassWrapper
		{
		public:
			MODIO_IMPL JavaClassWrapperModio(jobject Activity);

			MODIO_IMPL std::string GetCertificatePath();
			MODIO_IMPL std::string GetStoragePath();

		private:
			jmethodID GetCertificatePathMethodId;
			jmethodID GetStoragePathMethodId;
		};
	}; // namespace Detail

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "JavaClassWrapperModio.ipp"
#endif