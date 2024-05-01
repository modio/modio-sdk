/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#import "AppViewController.h"
#import "ModioSDKHelper.h"

NSString* const ModioAppIdentifier = @"ModioAppIdentifier";
NSString* const ModioUserIdentifier = @"ModioUserIdentifier";
NSString* const ModioEmailIdentifier = @"ModioEmailIdentifier";

@interface AppViewController ()

@property(nonatomic, retain) ASAuthorizationAppleIDButton* AppleIDButton;

- (void)CallSDKAuth:(NSString*)AppleToken Email:(NSString*)AppleEmail;
- (void)AuthCallback:(bool)AuthSuccess;
@end

@implementation AppViewController

@synthesize LoginInfoTextView;
@synthesize AppleIDButton;

- (void)viewDidLoad
{
	[super viewDidLoad];

	[ModioSDKHelper StartModioSDK];

	// Do any additional setup after loading the view.
	if (@available(iOS 13.0, *))
	{
		[self observeAppleSignInState];
		[self setupUI];
	}
}

- (void)viewDidAppear:(BOOL)animated
{
	[super viewDidAppear:animated];
	// [self perfomExistingAccountSetupFlows];
}

- (void)observeAppleSignInState
{
	if (@available(iOS 13.0, *))
	{
		NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
		[center addObserver:self
				   selector:@selector(handleSignInWithAppleStateChanged:)
					   name:ASAuthorizationAppleIDProviderCredentialRevokedNotification
					 object:nil];
	}
}

- (void)handleSignInWithAppleStateChanged:(id)noti
{
	NSLog(@"%s", __FUNCTION__);
	NSLog(@"%@", noti);
}

- (void)perfomExistingAccountSetupFlows
{
	if (@available(iOS 13.0, *))
	{
		// A mechanism for generating requests to authenticate users based on their Apple ID.
		ASAuthorizationAppleIDProvider* appleIDProvider = [ASAuthorizationAppleIDProvider new];

		// An OpenID authorization request that relies on the user’s Apple ID.
		ASAuthorizationAppleIDRequest* authAppleIDRequest = [appleIDProvider createRequest];

		// A mechanism for generating requests to perform keychain credential sharing.
		ASAuthorizationPasswordRequest* passwordRequest = [[ASAuthorizationPasswordProvider new] createRequest];

		NSMutableArray<ASAuthorizationRequest*>* mArr = [NSMutableArray arrayWithCapacity:2];
		if (authAppleIDRequest)
		{
			[mArr addObject:authAppleIDRequest];
		}
		if (passwordRequest)
		{
			[mArr addObject:passwordRequest];
		}
		// ASAuthorizationRequest：A base class for different kinds of authorization requests.
		NSArray<ASAuthorizationRequest*>* requests = [mArr copy];

		// A controller that manages authorization requests created by a provider.
		// Creates a controller from a collection of authorization requests.
		ASAuthorizationController* authorizationController =
			[[ASAuthorizationController alloc] initWithAuthorizationRequests:requests];

		// A delegate that the authorization controller informs about the success or failure of an authorization
		// attempt.
		authorizationController.delegate = self;
		// A delegate that provides a display context in which the system can present an authorization interface to the
		// user.
		authorizationController.presentationContextProvider = self;

		// starts the authorization flows named during controller initialization.
		[authorizationController performRequests];
	}
}

- (void)setupUI
{
	// Sign In With Apple

	LoginInfoTextView = [[UITextView alloc]
		initWithFrame:CGRectMake(.0, 100.0, CGRectGetWidth(self.view.frame), CGRectGetHeight(self.view.frame) * 0.4)
		textContainer:nil];

	LoginInfoTextView.font = [UIFont systemFontOfSize:16.0];

	[self.view addSubview:LoginInfoTextView];

	if (@available(iOS 13.0, *))
	{
		// Sign In With Apple Button
		AppleIDButton = [ASAuthorizationAppleIDButton new];

		AppleIDButton.frame = CGRectMake(.0, .0, CGRectGetWidth(self.view.frame) - 40.0, 100.0);
		CGPoint origin = CGPointMake(20.0, CGRectGetMidY(self.view.frame) + 20);
		CGRect frame = AppleIDButton.frame;
		frame.origin = origin;
		AppleIDButton.frame = frame;
		AppleIDButton.cornerRadius = CGRectGetHeight(AppleIDButton.frame) * 0.25;
		[self.view addSubview:AppleIDButton];
		[AppleIDButton addTarget:self
						  action:@selector(handleAuthrization:)
				forControlEvents:UIControlEventTouchUpInside];
	}

	NSMutableString* mStr = [NSMutableString string];
	[mStr appendString:@"Sign In to mod.io with your AppleID \n"];

	LoginInfoTextView.text = [mStr copy];
}

#pragma mark - Actions

- (void)handleAuthrization:(UIButton*)sender
{
	if (@available(iOS 13.0, *))
	{
		// A mechanism for generating requests to authenticate users based on their Apple ID.
		ASAuthorizationAppleIDProvider* appleIDProvider = [ASAuthorizationAppleIDProvider new];

		// Creates a new Apple ID authorization request.
		ASAuthorizationAppleIDRequest* request = appleIDProvider.createRequest;
		// The contact information to be requested from the user during authentication.
		request.requestedScopes = @[ASAuthorizationScopeFullName, ASAuthorizationScopeEmail];

		// A controller that manages authorization requests created by a provider.
		ASAuthorizationController* controller =
			[[ASAuthorizationController alloc] initWithAuthorizationRequests:@[request]];

		// A delegate that the authorization controller informs about the success or failure of an authorization
		// attempt.
		controller.delegate = self;

		// A delegate that provides a display context in which the system can present an authorization interface to the
		// user.
		controller.presentationContextProvider = self;

		// starts the authorization flows named during controller initialization.
		[controller performRequests];
	}
}

#pragma mark - Delegate

- (void)authorizationController:(ASAuthorizationController*)controller
	didCompleteWithAuthorization:(ASAuthorization*)authorization API_AVAILABLE(ios(13.0))
{
	NSString* AppleToken = nil;
	NSString* AppleEmail = nil;

	NSMutableString* mStr = [NSMutableString string];

	mStr = [LoginInfoTextView.text mutableCopy];

	if ([authorization.credential isKindOfClass:[ASAuthorizationAppleIDCredential class]])
	{
		// ASAuthorizationAppleIDCredential
		ASAuthorizationAppleIDCredential* AppleIDCredential = authorization.credential;
		NSString* User = AppleIDCredential.user;

		NSString* authorizationCode = [[NSString alloc] initWithData:AppleIDCredential.authorizationCode
															encoding:NSUTF8StringEncoding];

		NSLog(@"authorizationCode: %@", authorizationCode);

		AppleToken = [[NSString alloc] initWithData:AppleIDCredential.identityToken encoding:NSUTF8StringEncoding];

		NSLog(@"identityToken: %@", AppleToken);

		if (User != nil)
		{
			[[NSUserDefaults standardUserDefaults] setValue:User forKey:ModioUserIdentifier];
		}

		AppleEmail = AppleIDCredential.email;
		if (AppleEmail != nil)
		{
			[[NSUserDefaults standardUserDefaults] setValue:AppleEmail forKey:ModioEmailIdentifier];
		}

		[mStr appendString:User ?: @""];

		////          If necessary, you can try to retrieve more information from the user
		//		NSString* familyName = AppleIDCredential.fullName.familyName;
		//		[mStr appendString:familyName ?: @""];
		//		NSString* givenName = AppleIDCredential.fullName.givenName;
		//		[mStr appendString:givenName ?: @""];

		[mStr appendString:AppleEmail ?: @""];
		NSLog(@"mStr：%@", mStr);
		[mStr appendString:@"\n"];

		LoginInfoTextView.text = mStr;
	}
	else if ([authorization.credential isKindOfClass:[ASPasswordCredential class]])
	{
		ASPasswordCredential* passwordCredential = authorization.credential;
		NSString* user = passwordCredential.user;
		NSString* password = passwordCredential.password;
		[mStr appendString:user ?: @""];
		[mStr appendString:password ?: @""];
		[mStr appendString:@"\n"];
		NSLog(@"mStr：%@", mStr);

		LoginInfoTextView.text = mStr;
	}
	else
	{
		mStr = [@"check" mutableCopy];

		LoginInfoTextView.text = mStr;
	}

	[self CallSDKAuth:AppleToken Email:AppleEmail];
}

- (void)authorizationController:(ASAuthorizationController*)controller
		   didCompleteWithError:(NSError*)error API_AVAILABLE(ios(13.0))
{
	NSLog(@"error ：%@", error);
	NSString* errorMsg = nil;
	switch (error.code)
	{
		case ASAuthorizationErrorCanceled:
			errorMsg = @"ASAuthorizationErrorCanceled";
			break;
		case ASAuthorizationErrorFailed:
			errorMsg = @"ASAuthorizationErrorFailed";
			break;
		case ASAuthorizationErrorInvalidResponse:
			errorMsg = @"ASAuthorizationErrorInvalidResponse";
			break;
		case ASAuthorizationErrorNotHandled:
			errorMsg = @"ASAuthorizationErrorNotHandled";
			break;
		case ASAuthorizationErrorUnknown:
			errorMsg = @"ASAuthorizationErrorUnknown";
			break;
	}

	NSMutableString* mStr = [LoginInfoTextView.text mutableCopy];
	[mStr appendString:errorMsg];
	[mStr appendString:@"\n"];

	LoginInfoTextView.text = [mStr copy];

	if (errorMsg)
	{
		return;
	}

	if (error.localizedDescription)
	{
		NSMutableString* mStr = [LoginInfoTextView.text mutableCopy];
		[mStr appendString:error.localizedDescription];
		[mStr appendString:@"\n"];

		LoginInfoTextView.text = [mStr copy];
	}
}

//! Tells the delegate from which window it should present content to the user.
- (ASPresentationAnchor)presentationAnchorForAuthorizationController:(ASAuthorizationController*)controller
	API_AVAILABLE(ios(13.0))
{
	NSLog(@"window：%s", __FUNCTION__);
	return self.view.window;
}

- (void)dealloc
{
	if (@available(iOS 13.0, *))
	{
		[[NSNotificationCenter defaultCenter] removeObserver:self
														name:ASAuthorizationAppleIDProviderCredentialRevokedNotification
													  object:nil];
	}
}

#pragma mark - ModioSDK

- (void)CallSDKAuth:(NSString*)AppleToken Email:(NSString*)AppleEmail
{
	if (AppleToken == nil)
	{
		NSLog(@"SDK auth call needs an auth token");
		return;
	}
	// If we try to sign in again and there is no email, try to retrieve it from the user defaults
	if (AppleEmail == nil)
	{
		AppleEmail = [[NSUserDefaults standardUserDefaults] valueForKey:ModioEmailIdentifier];

		if (AppleEmail == nil)
		{
			NSLog(@"SDK auth call needs an email");
			return;
		}
	}

	[ModioSDKHelper AuthenticateWithAppleToken:AppleToken
										 Email:AppleEmail
                                  AuthCallback:^(bool Auth){ [self AuthCallback:Auth]; }];
}

- (void)AuthCallback:(bool)AuthSuccess
{
    if (AuthSuccess == true)
    {
        NSString *UserName = [ModioSDKHelper QueryUserName];
        
        dispatch_async(dispatch_get_main_queue(), ^() {
          [[self AppleIDButton] setHidden:true];

          NSMutableString* mStr = [LoginInfoTextView.text mutableCopy];
          [mStr appendFormat:@"User authenticated: %@", UserName];
          [mStr appendString:@"\n"];

          LoginInfoTextView.text = [mStr copy];
        });
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^() {
          [[self AppleIDButton] setHidden:false];

          NSMutableString* mStr = [LoginInfoTextView.text mutableCopy];
          [mStr appendString:@"User did not authenticate"];
          [mStr appendString:@"\n"];

          LoginInfoTextView.text = [mStr copy];
        });
    }
}

@end
