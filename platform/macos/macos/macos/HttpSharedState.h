/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once

#include "http/HttpRequestImplementation.h"
#include "modio/core/ModioErrorCode.h"
#include <CFNetwork/CFNetwork.h>
#include <CoreFoundation/CoreFoundation.h>
#include <memory>

namespace Modio
{
	namespace Detail
	{
		struct HttpSharedState
		{
			bool bCloseRequested = false;
			std::string UserAgentString;

			Modio::ErrorCode Initialize()
			{
				return {};
			}

			void InitializeRequest(std::shared_ptr<HttpRequestImplementation> Request, Modio::ErrorCode& ec)
			{
				// CoreNetwork HTTP Request Initialization
				CFStringRef Verb = CFStringCreateWithCString(kCFAllocatorDefault, Request->Parameters.GetVerb().c_str(),
															 kCFStringEncodingUTF8);

				// Construct URL for GET requests.
                // Because GetServerAddress provides the endpoint but not the protocol, URLStdStr stars
                // with https. Also, this is required by CF-HTTPRequest, not the CF-Socket
				std::string URLStdStr = "https://";
				URLStdStr += Request->Parameters.GetServerAddress();
				URLStdStr += Request->Parameters.GetFormattedResourcePath();
				CFStringRef URLStr =
					CFStringCreateWithCString(kCFAllocatorDefault, URLStdStr.c_str(), kCFStringEncodingUTF8);

				// Create a request URL string
				CFURLRef URLRef = CFURLCreateWithString(kCFAllocatorDefault, URLStr, NULL);
				// Create a HTTP Message Ref
				CFHTTPMessageRef RequestMessage =
					CFHTTPMessageCreateRequest(kCFAllocatorDefault, Verb, URLRef, kCFHTTPVersion1_1);
				Modio::Detail::HttpRequestParams::HeaderList HeaderLst = Request->Parameters.GetHeaders();

				// Append Headers to the HTTP Request
				for_each(HeaderLst.begin(), HeaderLst.end(),
						 [&RequestMessage](Modio::Detail::HttpRequestParams::Header Headr) {
							 CFStringRef Key = CFStringCreateWithCString(kCFAllocatorDefault, Headr.first.c_str(),
																		 kCFStringEncodingUTF8);
							 CFStringRef Value = CFStringCreateWithCString(kCFAllocatorDefault, Headr.second.c_str(),
																		   kCFStringEncodingUTF8);
							 CFHTTPMessageSetHeaderFieldValue(RequestMessage, Key, Value);
							 CFRelease(Value);
							 CFRelease(Key);
						 });

				if (Request->GetParameters().GetTypedVerb() == Verb::POST)
				{
					URLStdStr = Request->Parameters.GetServerAddress();
					CFStringRef HostURL =
						CFStringCreateWithCString(kCFAllocatorDefault, URLStdStr.c_str(), kCFStringEncodingUTF8);
					std::string PayloadStr = std::to_string(Request->GetParameters().GetPayloadSize());
					CFStringRef PayloadSize =
						CFStringCreateWithCString(kCFAllocatorDefault, PayloadStr.c_str(), kCFStringEncodingUTF8);
					// Set manually these HTTP headers
					CFHTTPMessageSetHeaderFieldValue(RequestMessage, CFSTR("Content-Length"), PayloadSize);
                    // It is important to have this header, specially for POST request that, once the
                    // server sends all the response on the CFStream, it should close the connection,
                    // that way the stream reaches an "end" state.
					CFHTTPMessageSetHeaderFieldValue(RequestMessage, CFSTR("Connection"), CFSTR("close"));
					CFHTTPMessageSetHeaderFieldValue(RequestMessage, CFSTR("Host"), HostURL);
					Request->HTTPRequestData = CFHTTPMessageCopySerializedMessage(RequestMessage);
					// Create the pair of streams
					CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault, HostURL, 443, &Request->ReadStream,
													   &Request->WriteStream);
					// It is necessary to enable SSL "manually" for Socket streams.
					CFReadStreamSetProperty(Request->ReadStream, kCFStreamPropertySocketSecurityLevel,
											kCFStreamSocketSecurityLevelNegotiatedSSL);
					CFWriteStreamSetProperty(Request->WriteStream, kCFStreamPropertySocketSecurityLevel,
											 kCFStreamSocketSecurityLevelNegotiatedSSL);
					// Release locally created objects
					CFRelease(PayloadSize);
					CFRelease(HostURL);
				}
                else
                {
                    // Create read stream objects
                    Request->ReadStream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, RequestMessage);
                    // Automatically handle redirections
                    CFReadStreamSetProperty(Request->ReadStream, kCFStreamPropertyHTTPShouldAutoredirect,
                                            kCFBooleanTrue);
                }

				// Release objects created in this function
				CFRelease(RequestMessage);
				CFRelease(URLRef);
				CFRelease(URLStr);
				CFRelease(Verb);
			}

			void Close()
			{
				bCloseRequested = true;
			}

			bool IsClosing()
			{
				return bCloseRequested;
			}
		};
	} // namespace Detail
} // namespace Modio
