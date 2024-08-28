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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/ModioOperationQueue.h"
#include "modio/file/ModioFile.h"
#include "modio/http/ModioHttpParams.h"

namespace Modio
{
	namespace Detail
	{
		struct PerformRequestImpl
		{
			Modio::Detail::OperationQueue::Ticket RequestTicket;
			std::unique_ptr<Modio::Detail::File> CurrentPayloadFile;
			Modio::Detail::DynamicBuffer PayloadFileBuffer;
			Modio::FileSize CurrentPayloadFileBytesRead;
			Modio::Optional<std::pair<std::string, Modio::Detail::PayloadContent>> PayloadElement;
			std::unique_ptr<Modio::Detail::Buffer> HeaderBuf;
			std::weak_ptr<Modio::ModProgressInfo> ProgressInfo;

		public:
			PerformRequestImpl(Modio::Detail::OperationQueue::Ticket RequestTicket)
				: RequestTicket(std::move(RequestTicket)) {}
			PerformRequestImpl(const PerformRequestImpl& Other) = delete;
		};
	} // namespace Detail
} // namespace Modio