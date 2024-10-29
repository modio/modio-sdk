/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#include <modio/ModioSDK.h>
#import "ModioSDKHelper.h"

bool FinishBackGroundThread = false;

@implementation ModioSDKHelper

+ (void)StartModioSDK:(SDKInitialization _Nonnull)Callback
{
	Modio::InitializeOptions InitOptions;
	InitOptions.APIKey = Modio::ApiKey("ca842a1f60c40bc8fb2044bc9932d763");
	InitOptions.GameEnvironment = Modio::Environment::Live;
	InitOptions.GameID = Modio::GameID(3609);
	InitOptions.User = "TestUser";
	
	////     Use the option below to add extra key/value pairs
	//    InitOptions.ExtendedParameters

    // Use "^()" blocks instead of "std::function" to interface between the mod.io SDK
    // and Swift
	Modio::InitializeAsync(InitOptions, ^(Modio::ErrorCode Err) {
		if (Err)
		{
			NSLog(@"Error during initalization: %s", Err.message().c_str());
            Callback(false);
			return;
		}

        Callback(true);
		NSLog(@"SDK initalization finished");
	});

    // Keep "RunPendingHandlers" running in the background
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
        while (FinishBackGroundThread != true)
        {
            Modio::RunPendingHandlers();
        }
	});
}

+ (void)ModioModList:(SDKListCallback _Nonnull)Callback
{
    Modio::FilterParams Filter;
    
    Modio::ListAllModsAsync(Filter, ^(Modio::ErrorCode Err, Modio::Optional<Modio::ModInfoList> List) {
        if (Err)
        {
            NSLog(@"Error during ShutdownAsync: %s", Err.message().c_str());
            return;
        }
        
        if (List.has_value() == false)
        {
            NSLog(@"No mods in the list");
            return;
        }
        
        // Parse the Modio::ModInfoList to Swift-compatible objects
        NSMutableArray *ModList = [NSMutableArray array];
        
        for (Modio::ModInfo Info : List.value())
        {
            NSMutableDictionary *ModInfo = [NSMutableDictionary dictionary];
            NSString *ProfileName = [NSString stringWithCString:Info.ProfileName.c_str() encoding:NSUTF8StringEncoding];
            NSString *ProfileDescription = [NSString stringWithCString:Info.ProfileDescriptionPlaintext.c_str() encoding:NSUTF8StringEncoding];
            NSNumber *ModID = [NSNumber numberWithLong:Info.ModId];
            [ModInfo setValue:ProfileName forKey:@"ProfileName"];
            [ModInfo setValue:ProfileDescription forKey:@"ProfileDescription"];
            [ModInfo setValue:ModID forKey:@"ModID"];
            [ModList addObject:ModInfo];
        }
        
        Callback(ModList);
    });
}

+ (void)ShutdownModioSDK
{
    // When you finish/exit the app, shutdown the SDK
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
@end
