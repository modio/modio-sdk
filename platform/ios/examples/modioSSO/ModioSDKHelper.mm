/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#import "ModioSDKHelper.h"

#include <modio/ModioSDK.h>

bool FinishBackGroundThread = false;
bool UserAuthenticated = false;

@interface ModioSDKHelper ()

@end

@implementation ModioSDKHelper

+ (void)StartModioSDK
{
	Modio::InitializeOptions InitOptions;
	InitOptions.APIKey = Modio::ApiKey("");
	InitOptions.GameEnvironment = Modio::Environment::Live;
	InitOptions.GameID = Modio::GameID(1);
	InitOptions.User = "TestUser";

	////     Use the option below to add extra key/value pairs
	//    InitOptions.ExtendedParameters

	Modio::InitializeAsync(InitOptions, [](Modio::ErrorCode Err) {
		if (Err)
		{
			NSLog(@"Error during initalization: %s", Err.message().c_str());
			return;
		}

		NSLog(@"SDK initalization finished");
	});

	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
	  while (FinishBackGroundThread != true)
	  {
		  Modio::RunPendingHandlers();
	  }
	});
}

+ (void)AuthenticateWithAppleToken:(NSString*)Token Email:(NSString*)Email AuthCallback:(SDKAuthCallback)Callback
{
	Modio::AuthenticationParams User;
	User.AuthToken = [Token cStringUsingEncoding:NSUTF8StringEncoding];
	User.UserEmail = [Email cStringUsingEncoding:NSUTF8StringEncoding];
	User.bUserHasAcceptedTerms = true;

    Modio::AuthenticateUserExternalAsync(User, Modio::AuthenticationProvider::Apple, ^(Modio::ErrorCode Err) {
		if (Err)
		{
			NSLog(@"Error during AuthenticateUserExternalAsync: %s", Err.message().c_str());
			Callback(false);
			return;
		}

		NSLog(@"User authenticated");
		Callback(true);
	});
}

+ (void)ShutdownModioSDK
{
	Modio::ShutdownAsync([](Modio::ErrorCode Err) {
		// Set flag to stop the while loop
		FinishBackGroundThread = true;

		if (Err)
		{
			NSLog(@"Error during ShutdownAsync: %s", Err.message().c_str());
			return;
		}
	});
}

+ (NSString *)QueryUserName
{
    Modio::Optional<Modio::User> ModioUser = Modio::QueryUserProfile();
    
    if (ModioUser.has_value() == false)
    {
        NSLog(@"No user found");
        return nil;
    }
    
    return [NSString stringWithCString:ModioUser.value().Username.c_str()
    encoding:NSUTF8StringEncoding];
}

@end
