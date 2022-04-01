/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once

#include "http/HttpRequestImplementation.h"
#include "macos/HttpSharedState.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"
#include <memory>

#include <CFNetwork/CFNetwork.h>
#include <CoreFoundation/CoreFoundation.h>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SendHttpRequestOp
		{
			asio::coroutine CoroutineState;
			std::shared_ptr<HttpRequestImplementation> Request;
			std::weak_ptr<HttpSharedState> SharedState;

		public:
			SendHttpRequestOp(std::shared_ptr<HttpRequestImplementation> Request,
							  std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				std::shared_ptr<HttpSharedState> PinnedState = SharedState.lock();

				if (PinnedState == nullptr || PinnedState->IsClosing())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				reenter(CoroutineState)
				{
					{
						Modio::ErrorCode InitStatus;
						PinnedState->InitializeRequest(Request, InitStatus);
						if (InitStatus)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
														"Init Request Failed");
							Self.complete(InitStatus);
							return;
						}

						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "Sending request",
													Request->GetParameters().GetFormattedResourcePath());
					}

					if (Request->GetParameters().GetTypedVerb() == Verb::POST)
					{
						if (CFWriteStreamOpen(Request->WriteStream) == false ||
							CFReadStreamOpen(Request->ReadStream) == false)
						{
							// This means that the ReadStream did not open
							CFStreamError StreamError = CFWriteStreamGetError(Request->WriteStream);

							if (StreamError.error != 0)
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
															"Could not open HTTPS POST connection, error code {}",
															StreamError.error);
								ec = Modio::make_error_code(Modio::HttpError::CannotOpenConnection);
							}

							CFWriteStreamClose(Request->WriteStream);
							Self.complete(ec);
							return;
						}
						else
						{
							// Write the headers if the request
							const UInt8* DataPtr = CFDataGetBytePtr(Request->HTTPRequestData);
							CFIndex DataLength = CFDataGetLength(Request->HTTPRequestData);
                            //// If debugging is necessary, this will print the headers attached to the HTTP request
                            //CFShow(CFStringCreateWithBytes(kCFAllocatorDefault, DataPtr, DataLength, kCFStringEncodingUTF8, false));
							CFWriteStreamWrite(Request->WriteStream, DataPtr, DataLength);

							// Some POST request might have a URL encoded Payload, which will not be called at
							// "WriteSomeToRequestOp" Therefore, it is sent at this stage.
							Modio::Optional<std::string> EncodedPayload =
								Request->GetParameters().GetUrlEncodedPayload();
							if (EncodedPayload.has_value() == true)
							{
								CFWriteStreamWrite(Request->WriteStream, (UInt8*) EncodedPayload.value().c_str(),
												   EncodedPayload.value().size());
							}
						}
					}
                    else
                    {
                        // An stream open means it commands CF to start the HTTPS request
                        if (CFReadStreamOpen(Request->ReadStream) == false)
                        {
                            // This means that the ReadStream did not open
                            CFStreamError StreamError = CFReadStreamGetError(Request->ReadStream);

                            if (StreamError.error != 0)
                            {
                                Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
                                                            "Could not open HTTPS connection, error code {}",
                                                            StreamError.error);
                                ec = Modio::make_error_code(Modio::HttpError::CannotOpenConnection);
                            }

                            CFReadStreamClose(Request->ReadStream);
                            Self.complete(ec);
                            return;
                        }
                    }

					Self.complete({});
				}
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
