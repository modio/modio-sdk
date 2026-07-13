/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#import "ios/AppleHttpSession.h"
#import "ios/AppleHttp_Internal.h"
#import "ios/AppleHttpRequest.h"

#import <Foundation/Foundation.h>

namespace ModioApple = Modio::Detail::Apple;

// Obj-C delegate forwarding callbacks to HttpSession::Impl.
// The raw pointer is safe because Close() blocks until
// didBecomeInvalidWithError: fires, which is guaranteed to be
// the last delegate message NSURLSession ever delivers.
@interface ModioHttpDelegate : NSObject <NSURLSessionDataDelegate>
{
	ModioApple::HttpSession::Impl* _SessionImpl;
}
- (instancetype)initWithSession:(ModioApple::HttpSession::Impl*)SessionImpl;
@end

@implementation ModioHttpDelegate

- (instancetype)initWithSession:(ModioApple::HttpSession::Impl*)SessionImpl
{
	self = [super init];
	if (self)
	{
		_SessionImpl = SessionImpl;
	}
	return self;
}

- (void)URLSession:(NSURLSession*)Session
		  dataTask:(NSURLSessionDataTask*)DataTask
didReceiveResponse:(NSURLResponse*)Response
 completionHandler:(void (^)(NSURLSessionResponseDisposition))CompletionHandler
{
	std::shared_ptr<ModioApple::HttpRequest::Impl> RequestImpl =
		_SessionImpl->LookupRequest(static_cast<std::uint64_t>(DataTask.taskIdentifier));
	if (RequestImpl == nullptr)
	{
		CompletionHandler(NSURLSessionResponseCancel);
		return;
	}

	std::uint32_t StatusCode = 0;
	std::vector<std::pair<std::string, std::string>> Headers;

	if ([Response isKindOfClass:[NSHTTPURLResponse class]])
	{
		NSHTTPURLResponse* HttpResponse = (NSHTTPURLResponse*) Response;
		StatusCode = static_cast<std::uint32_t>(HttpResponse.statusCode);
		Headers.reserve(HttpResponse.allHeaderFields.count);
		for (id Key in HttpResponse.allHeaderFields)
		{
			id Value = HttpResponse.allHeaderFields[Key];
			if ([Key isKindOfClass:[NSString class]] &&
				[Value isKindOfClass:[NSString class]])
			{
				Headers.emplace_back([(NSString*) Key UTF8String],
									 [(NSString*) Value UTF8String]);
			}
		}
	}

	RequestImpl->OnDidReceiveResponse(StatusCode, std::move(Headers));
	CompletionHandler(NSURLSessionResponseAllow);
}

- (void)URLSession:(NSURLSession*)Session
		  dataTask:(NSURLSessionDataTask*)DataTask
	didReceiveData:(NSData*)Data
{
	std::shared_ptr<ModioApple::HttpRequest::Impl> RequestImpl =
		_SessionImpl->LookupRequest(static_cast<std::uint64_t>(DataTask.taskIdentifier));
	if (RequestImpl == nullptr)
	{
		return;
	}
	// Avoids forcing a contiguous copy of dispatch_data-backed NSData.
	[Data enumerateByteRangesUsingBlock:
		^(const void* Bytes, NSRange ByteRange, BOOL* /*Stop*/) {
			RequestImpl->OnDidReceiveData(static_cast<const std::uint8_t*>(Bytes),
										  ByteRange.length);
		}];
}

- (void)URLSession:(NSURLSession*)Session
			  task:(NSURLSessionTask*)Task
didCompleteWithError:(NSError*)Error
{
	const std::uint64_t TaskId = static_cast<std::uint64_t>(Task.taskIdentifier);
	std::shared_ptr<ModioApple::HttpRequest::Impl> RequestImpl = _SessionImpl->LookupRequest(TaskId);
	if (RequestImpl != nullptr)
	{
		Modio::ErrorCode Translated;
		if (Error != nil)
		{
			Translated = ModioApple::TranslateUrlErrorCode(Error.code);
		}
		RequestImpl->OnDidComplete(Translated);
	}
	_SessionImpl->UnregisterTask(TaskId);
}

- (void)URLSession:(NSURLSession*)Session didBecomeInvalidWithError:(NSError*)Error
{
	// Final delegate message. Unblock Close().
	dispatch_semaphore_signal(_SessionImpl->InvalidationSemaphore);
}

@end

namespace Modio
{
	namespace Detail
	{
		namespace Apple
		{
			void HttpSession::Impl::RegisterRequest(std::uint64_t TaskIdentifier,
													std::weak_ptr<HttpRequest::Impl> Request)
			{
				std::lock_guard<std::mutex> Lock(MapMutex);
				TaskToRequest[TaskIdentifier] = std::move(Request);
			}

			void HttpSession::Impl::UnregisterTask(std::uint64_t TaskIdentifier)
			{
				std::lock_guard<std::mutex> Lock(MapMutex);
				TaskToRequest.erase(TaskIdentifier);
			}

			std::shared_ptr<HttpRequest::Impl> HttpSession::Impl::LookupRequest(std::uint64_t TaskIdentifier)
			{
				std::lock_guard<std::mutex> Lock(MapMutex);
				auto It = TaskToRequest.find(TaskIdentifier);
				if (It == TaskToRequest.end())
				{
					return nullptr;
				}
				// Returns nullptr if the owning HttpRequest was already destroyed.
				return It->second.lock();
			}

			HttpSession::HttpSession() : PImpl(std::make_shared<Impl>()) {}

			HttpSession::~HttpSession()
			{
				Close();
				PImpl->Session = nil;
				PImpl->Delegate = nil;
				PImpl->DelegateQueue = nil;
			}

			Modio::ErrorCode HttpSession::Initialize(const std::string& UserAgent)
			{
				PImpl->UserAgent = UserAgent;

				NSURLSessionConfiguration* Config =
					[NSURLSessionConfiguration defaultSessionConfiguration];
				Config.URLCache = nil;
				Config.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
				if (!UserAgent.empty())
				{
					Config.HTTPAdditionalHeaders = @{
						@"User-Agent" : [NSString stringWithUTF8String:UserAgent.c_str()]
					};
				}

				NSOperationQueue* Queue = [[NSOperationQueue alloc] init];
				Queue.maxConcurrentOperationCount = 1;
				Queue.name = @"io.mod.AppleHttpSession.delegate";
				PImpl->DelegateQueue = Queue;

				PImpl->Delegate = [[ModioHttpDelegate alloc] initWithSession:PImpl.get()];
				PImpl->Session = [NSURLSession sessionWithConfiguration:Config
															   delegate:PImpl->Delegate
														  delegateQueue:Queue];
				if (PImpl->Session == nil)
				{
					return Modio::make_error_code(Modio::HttpError::HttpNotInitialized);
				}
				return {};
			}

			std::unique_ptr<HttpRequest> HttpSession::CreateRequest()
			{
				return std::unique_ptr<HttpRequest>(new HttpRequest(*this));
			}

			void HttpSession::Close()
			{
				bool WasClosing = PImpl->Closing.exchange(true);
				if (WasClosing)
				{
					return;
				}
				if (PImpl->Session != nil)
				{
					// Cancels all in-flight tasks, then blocks until the
					// final delegate message (didBecomeInvalidWithError:)
					// has fired, ensuring no further callbacks reference Impl.
					[PImpl->Session invalidateAndCancel];
					dispatch_semaphore_wait(PImpl->InvalidationSemaphore, DISPATCH_TIME_FOREVER);
				}
			}

			bool HttpSession::IsClosing() const
			{
				return PImpl->Closing.load();
			}
		} // namespace Apple
	} // namespace Detail
} // namespace Modio