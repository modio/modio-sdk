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

typedef void(^SDKInitialization)(bool finished);
typedef void(^SDKListCallback)(NSArray * _Nonnull);

@interface ModioSDKHelper : NSObject

+ (void)StartModioSDK:(SDKInitialization _Nonnull)Callback;
+ (void)ModioModList:(SDKListCallback _Nonnull)Callback;
+ (void)ShutdownModioSDK;

@end
