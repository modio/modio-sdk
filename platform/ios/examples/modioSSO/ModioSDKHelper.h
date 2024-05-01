/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#include <Foundation/Foundation.h>

extern bool FinishBackGroundThread;
extern bool UserAuthenticated;

typedef void(^SDKAuthCallback)(bool Authenticated);

@interface ModioSDKHelper : NSObject

+ (void)StartModioSDK;
+ (void)AuthenticateWithAppleToken:(NSString*)Token Email:(NSString*)Email AuthCallback:(SDKAuthCallback)Callback;
+ (NSString *)QueryUserName;

@end
