/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#import "ios/AppleHttpRequest.h"
#import "ios/AppleHttp_Internal.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

#include <algorithm>
#include <cstring>

namespace Modio
{
	namespace Detail
	{
		namespace Apple
		{
			Modio::ErrorCode TranslateUrlErrorCode(long Code)
			{
				switch (Code)
				{
					case NSURLErrorCancelled:
						return Modio::make_error_code(Modio::GenericError::OperationCanceled);
					case NSURLErrorTimedOut:
					case NSURLErrorCannotFindHost:
					case NSURLErrorCannotConnectToHost:
					case NSURLErrorDNSLookupFailed:
					case NSURLErrorNotConnectedToInternet:
						return Modio::make_error_code(Modio::HttpError::CannotOpenConnection);
					case NSURLErrorNetworkConnectionLost:
						return Modio::make_error_code(Modio::HttpError::ServerClosedConnection);
					case NSURLErrorSecureConnectionFailed:
					case NSURLErrorServerCertificateHasBadDate:
					case NSURLErrorServerCertificateUntrusted:
					case NSURLErrorServerCertificateHasUnknownRoot:
					case NSURLErrorServerCertificateNotYetValid:
					case NSURLErrorClientCertificateRejected:
					case NSURLErrorClientCertificateRequired:
						return Modio::make_error_code(Modio::HttpError::SecurityConfigurationInvalid);
					case NSURLErrorHTTPTooManyRedirects:
						return Modio::make_error_code(Modio::HttpError::ExcessiveRedirects);
					case NSURLErrorBadServerResponse:
					case NSURLErrorZeroByteResource:
					case NSURLErrorCannotParseResponse:
						return Modio::make_error_code(Modio::HttpError::InvalidResponse);
					default:
						return Modio::make_error_code(Modio::HttpError::RequestError);
				}
			}

			// Impl state transitions

			bool HttpRequest::Impl::TryAdvanceStateLocked(HttpRequest::State Target)
			{
				// Caller must hold StateMutex.
				const HttpRequest::State Current = CurrentState.load(std::memory_order_relaxed);
				if (Current == HttpRequest::State::Cancelled ||
					Current == HttpRequest::State::Complete ||
					Current == HttpRequest::State::Failed)
				{
					return false;
				}
				CurrentState.store(Target, std::memory_order_release);
				return true;
			}

			// Impl callbacks (invoked on the delegate queue)

			void HttpRequest::Impl::OnDidReceiveResponse(
				std::uint32_t StatusCode,
				std::vector<std::pair<std::string, std::string>> Headers)
			{
				std::lock_guard<std::mutex> Lock(StateMutex);
				ResponseCode = StatusCode;
				ResponseHeaders = std::move(Headers);
				TryAdvanceStateLocked(HttpRequest::State::HeadersReceived);
			}

			void HttpRequest::Impl::OnDidReceiveData(const std::uint8_t* Bytes, std::size_t Length)
			{
				if (Length == 0 || Bytes == nullptr)
				{
					return;
				}
				Modio::Detail::Buffer Chunk(Length);
				std::memcpy(Chunk.Data(), Bytes, Length);
				std::lock_guard<std::mutex> Lock(StateMutex);
				// Discard chunks arriving after a terminal state.
				const HttpRequest::State Current = CurrentState.load(std::memory_order_relaxed);
				if (Current == HttpRequest::State::Cancelled ||
					Current == HttpRequest::State::Complete ||
					Current == HttpRequest::State::Failed)
				{
					return;
				}
				ReceivedBuffers.push_back(std::move(Chunk));
				ReceivedBytesPending += Length;
			}

			void HttpRequest::Impl::OnDidComplete(Modio::ErrorCode Error)
			{
				std::lock_guard<std::mutex> Lock(StateMutex);
				const HttpRequest::State Target =
					Error ? HttpRequest::State::Failed : HttpRequest::State::Complete;
				if (Error)
				{
					FinalError = Error;
				}
				TryAdvanceStateLocked(Target);
			}

			// HttpRequest public methods

			HttpRequest::HttpRequest(HttpSession& Owner) : PImpl(std::make_shared<Impl>(Owner.PImpl))
			{
				PImpl->Request = AdoptRef([[NSMutableURLRequest alloc] init]);
				PImpl->Request.Get().cachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
			}

			HttpRequest::~HttpRequest()
			{
				Cancel();
				// Remove the map entry. Late delegate callbacks that already
				// locked the shared_ptr keep Impl alive safely.
				if (PImpl->Task)
				{
					if (auto SessionImpl = PImpl->OwnerSession.lock())
					{
						SessionImpl->UnregisterTask(
							static_cast<std::uint64_t>(PImpl->Task.Get().taskIdentifier));
					}
				}
				if (PImpl->UploadWriteStream)
				{
					[PImpl->UploadWriteStream.Get() close];
					PImpl->UploadWriteStream.Reset();
				}
			}

			void HttpRequest::SetURL(const std::string& URL)
			{
				NSString* URLString = [NSString stringWithUTF8String:URL.c_str()];
				PImpl->Request.Get().URL = [NSURL URLWithString:URLString];
			}

			void HttpRequest::SetVerb(const std::string& Verb)
			{
				PImpl->Request.Get().HTTPMethod = [NSString stringWithUTF8String:Verb.c_str()];
			}

			void HttpRequest::AddHeader(const std::string& Name, const std::string& Value)
			{
				NSString* NameString = [NSString stringWithUTF8String:Name.c_str()];
				NSString* ValueString = [NSString stringWithUTF8String:Value.c_str()];
				[PImpl->Request.Get() setValue:ValueString forHTTPHeaderField:NameString];
			}

			void HttpRequest::SetBody(Modio::Detail::Buffer Body)
			{
				NSData* BodyData = [NSData dataWithBytes:Body.Data() length:Body.GetSize()];
				PImpl->Request.Get().HTTPBody = BodyData;
			}

			void HttpRequest::BeginStreamedBody(std::uint64_t TotalLength)
			{
				CFReadStreamRef ReadStream = NULL;
				CFWriteStreamRef WriteStream = NULL;
				static constexpr CFIndex BoundPairBufferSize = 64 * 1024;
				CFStreamCreateBoundPair(kCFAllocatorDefault, &ReadStream, &WriteStream, BoundPairBufferSize);

				// __bridge, unlike __bridge_transfer, is valid with and without ARC.
				// It transfers no ownership, so the +1 from CFStreamCreateBoundPair is
				// handed back below once the request and the handle have each taken a
				// reference of their own.
				NSInputStream* BodyInput = (__bridge NSInputStream*) ReadStream;
				NSOutputStream* BodyOutput = (__bridge NSOutputStream*) WriteStream;

				PImpl->Request.Get().HTTPBodyStream = BodyInput;
				PImpl->UploadWriteStream = BodyOutput;
				[PImpl->UploadWriteStream.Get() open];

				CFRelease(ReadStream);
				CFRelease(WriteStream);

				// Streamed bodies need an explicit Content-Length.
				NSString* LengthString = [NSString stringWithFormat:@"%llu",
											(unsigned long long) TotalLength];
				[PImpl->Request.Get() setValue:LengthString forHTTPHeaderField:@"Content-Length"];
			}

			void HttpRequest::Start()
			{
				std::shared_ptr<HttpSession::Impl> SessionImpl = PImpl->OwnerSession.lock();
				if (SessionImpl == nullptr || SessionImpl->Closing.load())
				{
					std::lock_guard<std::mutex> Lock(PImpl->StateMutex);
					PImpl->TryAdvanceStateLocked(State::Cancelled);
					return;
				}

				NSURLSession* Session = SessionImpl->Session.Get();

				if (PImpl->UploadWriteStream)
				{
					PImpl->Task = [Session uploadTaskWithStreamedRequest:PImpl->Request.Get()];
				}
				else
				{
					PImpl->Task = [Session dataTaskWithRequest:PImpl->Request.Get()];
				}

				// Register before resume so the delegate sees the task immediately.
				SessionImpl->RegisterRequest(
					static_cast<std::uint64_t>(PImpl->Task.Get().taskIdentifier),
					std::weak_ptr<Impl>(PImpl));

				bool Advanced = false;
				{
					std::lock_guard<std::mutex> Lock(PImpl->StateMutex);
					Advanced = PImpl->TryAdvanceStateLocked(State::InFlight);
				}
				if (!Advanced)
				{
					// A concurrent Cancel() already moved to a terminal state.
					[PImpl->Task.Get() cancel];
					return;
				}
				[PImpl->Task.Get() resume];
			}

			void HttpRequest::Cancel()
			{
				bool Advanced = false;
				{
					std::lock_guard<std::mutex> Lock(PImpl->StateMutex);
					Advanced = PImpl->TryAdvanceStateLocked(State::Cancelled);
				}
				if (!Advanced)
				{
					return;
				}
				if (PImpl->Task)
				{
					[PImpl->Task.Get() cancel];
				}
				if (PImpl->UploadWriteStream)
				{
					[PImpl->UploadWriteStream.Get() close];
				}
			}

			HttpRequest::State HttpRequest::GetState() const
			{
				return PImpl->CurrentState.load(std::memory_order_acquire);
			}

			Modio::ErrorCode HttpRequest::GetError() const
			{
				std::lock_guard<std::mutex> Lock(PImpl->StateMutex);
				return PImpl->FinalError;
			}

			std::uint32_t HttpRequest::GetResponseCode() const
			{
				std::lock_guard<std::mutex> Lock(PImpl->StateMutex);
				return PImpl->ResponseCode;
			}

			std::vector<std::pair<std::string, std::string>> HttpRequest::GetResponseHeaders() const
			{
				std::lock_guard<std::mutex> Lock(PImpl->StateMutex);
				return PImpl->ResponseHeaders;
			}

			std::size_t HttpRequest::DrainResponseBody(std::uint8_t* Out, std::size_t MaxBytes)
			{
				if (Out == nullptr || MaxBytes == 0)
				{
					return 0;
				}
				std::lock_guard<std::mutex> Lock(PImpl->StateMutex);
				std::size_t Written = 0;
				while (Written < MaxBytes && !PImpl->ReceivedBuffers.empty())
				{
					Modio::Detail::Buffer& Front = PImpl->ReceivedBuffers.front();
					const std::size_t Available = Front.GetSize();
					const std::size_t Take = std::min(Available, MaxBytes - Written);
					std::memcpy(Out + Written, Front.Data(), Take);
					Written += Take;
					if (Take == Available)
					{
						PImpl->ReceivedBuffers.pop_front();
					}
					else
					{
						// Partial drain: Keep the remaining tail
						Modio::Detail::Buffer Remainder(Available - Take);
						std::memcpy(Remainder.Data(), Front.Data() + Take, Available - Take);
						PImpl->ReceivedBuffers.front() = std::move(Remainder);
					}
				}
				PImpl->ReceivedBytesPending -= Written;
				return Written;
			}

			std::size_t HttpRequest::DrainResponseBody(Modio::Detail::DynamicBuffer& Out, std::size_t MaxBytes)
			{
				std::lock_guard<std::mutex> Lock(PImpl->StateMutex);

				const bool Unlimited = (MaxBytes == 0);
				std::size_t Total = 0;

				while (!PImpl->ReceivedBuffers.empty())
				{
					Modio::Detail::Buffer& Front = PImpl->ReceivedBuffers.front();
					const std::size_t Available = Front.GetSize();

					if (!Unlimited && Total + Available > MaxBytes)
					{
						const std::size_t Take = MaxBytes - Total;
						const std::size_t Rest = Available - Take;

						Modio::Detail::Buffer Partial(Take);
						std::memcpy(Partial.Data(), Front.Data(), Take);
						Out.AppendBuffer(std::move(Partial));

						Modio::Detail::Buffer Remainder(Rest);
						std::memcpy(Remainder.Data(), Front.Data() + Take, Rest);
						PImpl->ReceivedBuffers.front() = std::move(Remainder);

						Total += Take;
						break;
					}

					Total += Available;
					Out.AppendBuffer(std::move(Front));
					PImpl->ReceivedBuffers.pop_front();
				}

				PImpl->ReceivedBytesPending -= Total;
				return Total;
			}

			bool HttpRequest::HasBufferedBody() const
			{
				std::lock_guard<std::mutex> Lock(PImpl->StateMutex);
				return !PImpl->ReceivedBuffers.empty();
			}

			std::size_t HttpRequest::WriteRequestBody(const std::uint8_t* Bytes, std::size_t Length)
			{
				if (!PImpl->UploadWriteStream || Bytes == nullptr || Length == 0)
				{
					return 0;
				}
				NSInteger Written = [PImpl->UploadWriteStream.Get() write:Bytes maxLength:Length];
				if (Written < 0)
				{
					return 0;
				}
				return static_cast<std::size_t>(Written);
			}

			std::size_t HttpRequest::WriteRequestBody(const Modio::Detail::Buffer& Data)
			{
				return WriteRequestBody(Data.Data(), Data.GetSize());
			}

			bool HttpRequest::CanAcceptBodyBytes() const
			{
				if (!PImpl->UploadWriteStream)
				{
					return false;
				}
				return [PImpl->UploadWriteStream.Get() hasSpaceAvailable] == YES;
			}
		} // namespace Apple
	} // namespace Detail
} // namespace Modio