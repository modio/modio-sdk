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
#include "ModioGeneratedVariables.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/compression/zip/CompressionImplementation.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
		class CompressionService : public asio::detail::service_base<CompressionService>
		{
			// TODO: @Modio-Core Better name for Compression/Compressor Implementation?
			std::shared_ptr<CompressionImplementation> PlatformImplementation;

		public:
			explicit CompressionService(asio::io_context& IOService)
				: asio::detail::service_base<CompressionService>(IOService)
			{
				auto NewImplementation = std::make_shared<CompressionImplementation>();
				PlatformImplementation.swap(NewImplementation);
			}

			~CompressionService() {}

			using implementation_type = CompressionImplementation::IOObjectImplementationType;

			MODIO_IMPL void construct(implementation_type& Implementation);

			MODIO_IMPL void destroy(implementation_type& Implementation) {}

			MODIO_IMPL void move_construct(implementation_type& Implementation, implementation_type& Other);

			MODIO_IMPL void move_assign(implementation_type& Implementation,
										Modio::Detail::CompressionService& OtherService, implementation_type& Other);

			MODIO_IMPL void converting_move_construct(implementation_type& impl, Modio::Detail::CompressionService&,
													  implementation_type& other_impl);

			MODIO_IMPL void converting_move_assign(implementation_type& impl,
												   Modio::Detail::CompressionService& other_service,
												   implementation_type& other_impl);

			template<typename CompletionHandlerType>
			auto AddFileEntryAsync(implementation_type& PlatformIOObject, Modio::filesystem::path SourceFilePath,
								   Modio::filesystem::path PathInsideArchive, std::weak_ptr<class Modio::ModProgressInfo> ProgressInfo, CompletionHandlerType&& Handler)
			{
				PlatformImplementation->AddFileEntryAsync(PlatformIOObject, SourceFilePath, PathInsideArchive, ProgressInfo,
														  std::forward<CompletionHandlerType>(Handler));
			}

			template<typename CompletionHandlerType>
			auto AddDirectoryEntryAsync(implementation_type& PlatformIOObject, Modio::filesystem::path DirectoryPath,
										CompletionHandlerType&& Handler)
			{
				PlatformImplementation->AddDirectoryEntryAsync(PlatformIOObject, DirectoryPath,
															   std::forward<CompletionHandlerType>(Handler));
			}

			template<typename CompletionHandlerType>
			auto FinalizeArchiveAsync(implementation_type& PlatformIOObject, CompletionHandlerType&& Handler)
			{
				PlatformImplementation->FinalizeArchiveAsync(PlatformIOObject,
															 std::forward<CompletionHandlerType>(Handler));
			}

			//ToDO: @modio-core add the archive reading functions here so they can live on the compression implementation rather than the ArchiveReader
		private:
			void shutdown_service() {}
		};
	} // namespace Detail

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioCompressionService.ipp"
#endif