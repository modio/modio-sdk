/*
 *  Copyright (C) 2020-2022 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#include "modio/ModioSDK.h"

#include <iostream>
#include <utility>

/// This example extends 01_Initialization.cpp by performing email authentication. It begins
/// by performing an asynchronous initialization of the SDK, and if that is successful will prompt the user to input an
/// email address. The email address will be submitted to the mod.io REST API, and if validation on the email address is
/// successful an email will be sent to the specified address containing a security code, which the application will
/// prompt the user for. This code will then be sent to the mod.io REST API, and if validated will result in a mod.io
/// user profile being stored in the UserSession which you provided to InitializeAsync.
/// Once authentication has succeeded or failed, the sample initiates an async shutdown of the SDK if necessary and then
/// terminates.

/// @brief Simple struct containing state variables for the example
struct ModioExampleFlags
{
	/// @brief Variable simulating a notification passed from a callback indicating the callback was invoked (ie the
	/// async SDK function returned a result)
	bool bDone = false;

	/// @brief Variable simulating a notification passed from a callback indicating whether the SDK function returned a
	/// success or failure
	bool bSuccess = false;

	/// @brief Keeps a reference to the last error received
	Modio::ErrorCode LastError;
};

/// @brief Static definition of the ModioExampleFlags. Consider a more sophisticated process
/// to deal with asynchronous calls should be designed in your game
static ModioExampleFlags Example;

/// @brief Static definition of the ModioExampleFlags. Consider a more sophisticated process
/// to deal with asynchronous calls should be designed in your game
struct ModioExample
{
	static bool HasAsyncOperationCompleted()
	{
		return std::exchange(Example.bDone, false);
	}

	static bool DidAsyncOperationSucceed()
	{
		return std::exchange(Example.bSuccess, false);
	}

	static void NotifyApplicationSuccess()
	{
		Example.bDone = true;
		Example.bSuccess = true;
		Example.LastError = {};
	}

	static void NotifyApplicationFailure(Modio::ErrorCode ec)
	{
		Example.bDone = true;
		Example.bSuccess = false;
		Example.LastError = ec;
		// More information about which error occurred can be retrieved with the message()
		// function
		std::cout << ec.message() << std::endl;
	}

	static void OnInitializeComplete(Modio::ErrorCode ec)
	{
		// The ErrorCode value passed back via the callback will be implicitly convertible to
		// true if an error occurred
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			NotifyApplicationSuccess();
		}
	}

	/// @brief User callback invoked by Modio::RunPendingHandlers() when Modio::RequestEmailAuthCodeAsync has completed
	/// @param ec Error code indicating the success or failure of RequestEmailAuthCodeAsync
	static void OnRequestEmailAuthCodeCompleted(Modio::ErrorCode ec)
	{
		// The ErrorCode value passed back via the callback will be implicitly convertible to
		// true if an error occurred
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			NotifyApplicationSuccess();
		}
	}

	/// @brief User callback invoked by Modio::RunPendingHandlers() when Modio::AuthenticateUserEmailAsync has completed
	/// @param ec Error code indicating the success or failure of AuthenticateUserEmailAsync
	static void OnAuthenticateUserEmailCompleted(Modio::ErrorCode ec)
	{
		// The ErrorCode value passed back via the callback will be implicitly convertible to
		// true if an error occurred
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			NotifyApplicationSuccess();
		}
	}

	static void OnShutdownComplete(Modio::ErrorCode ec)
	{
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			NotifyApplicationSuccess();
		}
	}
};

int main()
{
	Modio::InitializeAsync(Modio::InitializeOptions(Modio::GameID(3609),
													Modio::ApiKey("ca842a1f60c40bc8fb2044bc9932d763"),
													Modio::Environment::Live, Modio::Portal::None, "ExampleSession"),
						   ModioExample::OnInitializeComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	// Modio::InitializeAsync completed, but we now need to check if it resulted in a success or a failure
	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// SDK initialization failed - this simple sample returns immediately, no need to call ShutdownAsync
		return -1;
	}

	{
		// Prompt user for input here
		std::string UserEmailAddress;
		std::cout << "Enter email address:" << std::endl;
		std::getline(std::cin, UserEmailAddress);

		// Email authentication with mod.io is a two-step process. First, a user email address is submitted via
		// RequestEmailAuthCodeAsync, which triggers an email containing an authentication code to that address.
		Modio::RequestEmailAuthCodeAsync(Modio::EmailAddress(UserEmailAddress),
										 ModioExample::OnRequestEmailAuthCodeCompleted);
	}

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// It is possible to check the error type and identify what next steps to follow
		if (Modio::ErrorCodeMatches(Example.LastError, Modio::ErrorConditionTypes::NetworkError)) // NetworkError group
        {
            // Error code represents some network error kind. Possibly ask the user to try again later.
			std::cout << "A network error" << std::endl;
        }
        else if (Modio::ErrorCodeMatches(Example.LastError, Modio::ErrorConditionTypes::EntityNotFoundError)) //Entity Not Found group
        {
            // An mod entity is not located with this configuration. Therefore, the list you're fetching the ModID from is probably stale. A remedy could be to fetch an updated version of the list from the server.
			std::cout << "The entity requested was not found" << std::endl;
        }
        else if (Modio::ErrorCodeMatches(Example.LastError, Modio::GenericError::OperationCanceled)) // SDK Operation cancelled
        {
            // Your application cancelled a SDK function
			std::cout << "SDK operation cancelled" << std::endl;
        }

		// We initialized the SDK, but received an error when requesting an email auth code. Begin async shutdown
		Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

		// Wait for async shutdown to complete
		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		};

		// Bail without running any other calls
		return -1;
	}

	{
		// Prompt user for input here
		std::string UserEmailAuthCode;
		std::cout << "Enter email auth code:" << std::endl;
		std::getline(std::cin, UserEmailAuthCode);
		// The second part of email authentication with the mod.io SDK involves submitting the user's email
		// authentication code. To do this, you will need to prompt your user for input and then pass that input to
		// AuthenticateUserEmailAsync. If this function returns successfully, the mod.io account will be stored in the
		// Local User Session passed into InitializeAsync earlier.
		Modio::AuthenticateUserEmailAsync(Modio::EmailAuthCode(UserEmailAuthCode),
										  ModioExample::OnAuthenticateUserEmailCompleted);
	}

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// This sample is going to shut down regardless of success or failure, so no need to do any handling of
		// authentication errors here
	}

	// Request the SDK begins an async shutdown by invoking Modio::ShutdownAsync
	// Parameters:
	// Callback Callback invoked when ShutdownAsync has completed. It takes a single ErrorCode parameter
	// indicating whether an error occurred.
	Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// No need for explicit handling of an error from ShutdownAsync, the application is about to exit
	}
}
