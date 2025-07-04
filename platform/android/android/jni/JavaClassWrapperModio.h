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
			MODIO_IMPL JavaClassWrapperModio(jobject Activity, bool bUseExternalStorageForMods);

			MODIO_IMPL std::string GetCertificatePath();
			MODIO_IMPL std::string GetInternalStoragePath();
			MODIO_IMPL std::string GetExternalStoragePath();

		private:
			jmethodID GetCertificatePathMethodId {};
			jmethodID GetInternalStoragePathMethodId {};
			jmethodID GetExternalStoragePathMethodId {};
		};
	}; // namespace Detail

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "JavaClassWrapperModio.ipp"
#endif