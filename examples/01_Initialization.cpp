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
#include <sstream>
#include <utility>

/// This example shows SDK initialization and shutdown procedures. It begins an asynchronous call that initializes the
/// SDK, and when initialization is complete, if that process was successful, it begins an asynchronous shutdown and
/// waits for that to complete before exiting.

/// @brief Simple struct containing state variables for the example
struct ModioExampleFlags
{
	/// @brief Variable simulating a notification passed from a callback indicating the callback was invoked (ie the
	/// async SDK function returned a result)
	bool bDone = false;

	/// @brief Variable simulating a notification passed from a callback indicating whether the SDK function returned a
	/// success or failure
	bool bSuccess = false;
};

/// @brief Static definition of the ModioExampleFlags. Consider a more sophisticated process
/// to deal with asynchronous calls should be designed in your game
static ModioExampleFlags Example;

/// @brief Simple struct containing callbacks and minimal state variables for the example
struct ModioExample
{
	/// @brief Function simplifying checking the completed flag by using std::exchange to ensure it's always reset back
	/// to false after being queried. This means we don't need to manually reset it if we're calling multiple async
	/// methods in a sample
	/// @return Previous value of Example.bDone, which will be true if an operation finished/completed
	static bool HasAsyncOperationCompleted()
	{
		return std::exchange(Example.bDone, false);
	}

	/// @brief Function simplifying checking the success flag by using std::exchange to ensure it's always reset back to
	/// false after being queried. This means we don't need to manually reset it if we're calling multiple async methods
	/// in a sample
	/// @return Previous value of Example.bSuccess, which will be true if a) an operation finished/completd AND b) the
	/// operation completed successfully
	static bool DidAsyncOperationSucceed()
	{
		return std::exchange(Example.bSuccess, false);
	}

	/// @brief Method that simulates the callback you gave to the SDK notifying your application of a failed async call
	static void NotifyApplicationSuccess()
	{
		Example.bDone = true;
		Example.bSuccess = true;
	}

	/// @brief Method that simulates the callback you gave to the SDK notifying your application of a failed async call
	/// @param ec The error that occurred
	static void NotifyApplicationFailure(Modio::ErrorCode ec)
	{
		Example.bDone = true;
		Example.bSuccess = false;

		// More information about which error occurred can be retrieved with the message()
		// function
        std::cout << ec.message() << std::endl;
	}

	/// @brief User callback invoked by Modio::RunPendingHandlers() when Modio::InitializeAsync has completed
	/// @param ec Error code indicating the success or failure of InitializeAsync
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

	/// @brief User callback invoked by Modio::RunPendingHandlers() when Modio::ShutdownAsync has completed
	/// @param ec Error code indicating the success or failure of ShutdownAsync
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

	/// @brief Helper method to retrieve input from a user with an string prompt
	/// @param Prompt The string to show related to the expected user input
	/// @param DefaultValue If the user wants to use a default input, this will be used
	/// @return String with the user input or default value
	static std::string RetrieveUserInput(std::string Prompt, std::string DefaultValue = "")
	{
		std::string UserInput = "";
		std::cout << Prompt << std::endl;

		if (DefaultValue.size() != 0)
		{
			std::cout << "(To use the default value \"" << DefaultValue << "\", just tap Enter)" << std::endl;
		}
		std::getline(std::cin, UserInput);

		if (UserInput.size() == 0)
		{
			return DefaultValue;
		}
		else
		{
			return UserInput;
		}
	}
};

int main()
{
	// Ask user for their input and offer default values
	std::stringstream IntTransformer;
	IntTransformer << ModioExample::RetrieveUserInput("Game ID:", "3609");
	std::string APIStr = ModioExample::RetrieveUserInput("API key:", "ca842a1f60c40bc8fb2044bc9932d763");
	std::string ModioEnvStr = ModioExample::RetrieveUserInput("Modio Environment:", "Live");
	std::string SessionID = ModioExample::RetrieveUserInput("SessionID:", "ExampleSession");

	// Transform GameID to integer
	int GameIDInt = 0;
	IntTransformer >> GameIDInt;

	// Determine the modio API environment to use
	Modio::Environment ModioEnv;

	if (ModioEnvStr == "Test" || ModioEnvStr == "test")
	{
		ModioEnv = Modio::Environment::Test;
	}
	else
	{
		ModioEnv = Modio::Environment::Live;
	}

	// Perform async initialization of the SDK
	// Parameters:
	// GameID Your game's ID, retrievable from the dashboard at https://mod.io
	// ApiKey Your game's API key, retrievable from the dashboard at  https://mod.io/apikey
	// Environment Is your game on the test server or the live server?
	// Portal The store or service your game is being distributed through
	// SessionID String identifier for the user session you'd like to initialize. Each user session acts as a container
	// for a potentially authenticated user.
	// Callback Function to invoke when the initialization process succeeds. This can be anything convertible to
	// std::function with the correct signature, i.e lambda function, static member function, non-static member function
	// (with `this` bound via std::bind or invoked via a capturing lambda), or free function
	Modio::InitializeAsync(Modio::InitializeOptions(Modio::GameID(GameIDInt), Modio::ApiKey(APIStr), ModioEnv,
													Modio::Portal::None, SessionID),
						   ModioExample::OnInitializeComplete);

	// The SDK operates an internal event loop that handles all of the work it performs. InitializeAsync queues the
	// initialization 'operation', but returns immediately, so we must run that event loop in order to actually perform
	// the underlying work. To run the event loop, invoke Modio::RunPendingHandlers.
	// To simplify sequencing async calls in these examples and make their ordering easier to follow, we are simulating
	// blocking calls to each SDK function by immediately invoking RunPendingHandlers until the async call invokes the
	// user callback. In your own application, you would call RunPendingHandlers during your own event loop, and your
	// callbacks would emit events or notifications so that you can begin the next call, as necessary.
	//
	// The SDK guarantees that all internal work, and all callbacks, will only be run on the thread
	// calling RunPendingHandlers. Because the SDK is currently not thread-safe, this means you should be calling all
	// SDK functions on this same thread. However, this has the advantage of ensuring that there's no synchronization
	// overhead.
	// You must continue to invoke Modio::RunPendingHandlers whilst the SDK is shutting down via a call to
	// Modio::ShutdownAsync.

	// Using std::exchange here ensures that we don't need to manually reset Example.bDone after each call
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

	// Request the SDK begins an async shutdown by invoking Modio::ShutdownAsync
	// Parameters:
	// Callback Callback invoked when ShutdownAsync has completed. It takes a single ErrorCode parameter
	// indicating whether an error occurred.
	Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	// No need for explicit handling of an error in ShutdownAsync, the application is about to exit
}
