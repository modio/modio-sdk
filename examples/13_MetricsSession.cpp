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

/// This example demonstrates how to start a new metrics session, perform a heartbeat and shut down the session
/// Once these queries have been performed, the sample initiates an async shutdown of the SDK if necessary and then
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

	/// @brief Keeps a reference to the list of mod results
	Modio::Optional<Modio::ModInfoList> LastResults;

	/// @brief Keeps a reference to the last mod management event
	Modio::ModManagementEvent LastEvent;
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

	static void MetricsSessionStartCallback(Modio::ErrorCode ec)
	{
		if (ec)
		{
			std::cout << "Metrics session failed to start. Error occurred." << std::endl;
			NotifyApplicationFailure(ec);
		}
		else
		{
			std::cout << "Metrics session successfully started" << std::endl;
			NotifyApplicationSuccess();
		}
	}

	static void MetricsSessionHeartbeatCallback(Modio::ErrorCode ec)
	{
		if (ec)
		{
			std::cout << "Metrics session failed to submit a heartbeat. Error occurred." << std::endl;
			NotifyApplicationFailure(ec);
		}
		else
		{
			std::cout << "Metrics session heartbeat successfully submitted" << std::endl;
			NotifyApplicationSuccess();
		}
	}

	static void MetricsSessionEndCallback(Modio::ErrorCode ec)
	{
		if (ec)
		{
			std::cout << "Metrics session failed to shutdown. Error occurred." << std::endl;
			NotifyApplicationFailure(ec);
		}
		else
		{
			std::cout << "Metrics session successfully shut down" << std::endl;
			NotifyApplicationSuccess();
		}
	}

	static void OnListAllModsComplete(Modio::ErrorCode ec, Modio::Optional<Modio::ModInfoList> ModList)
	{
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			NotifyApplicationSuccess();
		}

		Example.LastResults = ModList;
	}

	/// @brief Helper method to retrieve input from a user with an string prompt
	/// @param Prompt The string to show related to the expected user input
	/// @param DefaultValue If the user wants to use a default input, this will be used
	/// @return String with the user input or default value
	static std::string RetrieveUserInput(std::string Prompt, std::string DefaultValue = std::string())
	{
		std::string UserInput;
		std::cout << Prompt << std::endl;

		if (!DefaultValue.empty())
		{
			std::cout << "(To use the default value \"" << DefaultValue << "\", just tap Enter)" << std::endl;
		}
		std::getline(std::cin, UserInput);

		if (UserInput.empty())
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
	std::cout << "Starting example for Metrics Session" << std::endl;
	// Ask user for their input and offer default values
	std::stringstream IntTransformer;
	std::string UserInput = ModioExample::RetrieveUserInput("Game ID:", "3609");
	IntTransformer << UserInput;
	std::string APIStr = ModioExample::RetrieveUserInput("API key:", "ca842a1f60c40bc8fb2044bc9932d763");
	std::string MetricsStr =
		ModioExample::RetrieveUserInput("Metrics Secret key:", "00000000-1111-2222-3333-444444444444");
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

	auto Params = Modio::InitializeOptions(Modio::GameID(GameIDInt), Modio::ApiKey(APIStr), ModioEnv,
										   Modio::Portal::None, SessionID);

	// Set the metrics secret key up in the extended params
	Params.ExtendedParameters["MetricsSecretKey"] = MetricsStr;

	Modio::InitializeAsync(Params, ModioExample::OnInitializeComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	}

	// Modio::InitializeAsync completed, but we now need to check if it resulted in a success or a failure
	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// SDK initialization failed - this simple sample returns immediately, no need to call ShutdownAsync
		return -1;
	}

	{
		// Prompt user for input here
		std::string UserEmailAddress = ModioExample::RetrieveUserInput("Enter email address:");

		// Email authentication with mod.io is a two-step process. First, a user email address is submitted via
		// RequestEmailAuthCodeAsync, which triggers an email containing an authentication code to that address.
		Modio::RequestEmailAuthCodeAsync(Modio::EmailAddress(UserEmailAddress),
										 ModioExample::OnRequestEmailAuthCodeCompleted);
	}

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	}

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// It is possible to check the error type and identify what next steps to follow
		if (Modio::ErrorCodeMatches(Example.LastError, Modio::ErrorConditionTypes::NetworkError)) // NetworkError group
		{
			// Error code represents some network error kind. Possibly ask the user to try again later.
			std::cout << "A network error" << std::endl;
		}
		else if (Modio::ErrorCodeMatches(Example.LastError,
										 Modio::ErrorConditionTypes::EntityNotFoundError)) // Entity Not Found group
		{
			// An mod entity is not located with this configuration. Therefore, the list you're fetching the ModID from
			// is probably stale. A remedy could be to fetch an updated version of the list from the server.
			std::cout << "The entity requested was not found" << std::endl;
		}
		else if (Modio::ErrorCodeMatches(Example.LastError,
										 Modio::GenericError::OperationCanceled)) // SDK Operation cancelled
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
		}

		// Bail without running any other calls
		return -1;
	}

	{
		// Prompt user for input here
		std::string UserEmailAuthCode = ModioExample::RetrieveUserInput("Enter email auth code:");

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
	}

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// Perform any auth error handling here as appropriate.
	}

	// Search for mods so that we can add them to a session
	// Generally speaking you should already have a list of the mods to add to a session based on the context of your
	// game
	{
		Modio::ListAllModsAsync({}, ModioExample::OnListAllModsComplete);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		}

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			std::cout << "Call failed" << std::endl;
		}
		else
		{
			if (Example.LastResults.has_value())
			{
				std::cout << "Result count:" << Example.LastResults->GetResultCount() << std::endl;
				for (const Modio::ModInfo& CurrentModInfo : Example.LastResults.value())
				{
					std::cout << CurrentModInfo.ModId << "\t" << CurrentModInfo.ProfileName << std::endl;
				}
			}
			else
			{
				std::cout << "Error, no mods were found for this game" << std::endl;
				return -1;
			}
		}
	}

	// Select which mod(s) to add to a session
	std::int64_t UserModID = Example.LastResults.value()[0].ModId;
	{
		std::string UserModString =
			ModioExample::RetrieveUserInput("Enter the ID of the mod to add to a session:", std::to_string(UserModID));

		UserModID = std::stoll(UserModString);
		if (UserModID < 0)
		{
			std::cout << "Invalid Input" << std::endl;
			return -1;
		}
		std::cout << "Mod ID will be added to the session:" << UserModID << std::endl;
	}

	// Start a new metrics session with our session parameters
	{
		std::cout << "Starting a new metrics session" << std::endl;
		Modio::MetricsSessionParams SessionParams {{}, std::vector {Modio::ModID(UserModID)}};

		Modio::MetricsSessionStartAsync(SessionParams, ModioExample::MetricsSessionStartCallback);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		}

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// Error handling for MetricsSessionStartAsync errors
		}
	}

	// Send a heartbeat to keep the session alive
	{
		std::cout << "Sending a metrics session heartbeat once" << std::endl;
		Modio::MetricsSessionSendHeartbeatOnceAsync(ModioExample::MetricsSessionHeartbeatCallback);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		}

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// Error handling for MetricsSessionSendHeartbeatOnceAsync errors
		}
	}

	// End the metrics session
	{
		std::cout << "Shutting down the metrics session" << std::endl;
		Modio::MetricsSessionEndAsync(ModioExample::MetricsSessionEndCallback);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		}

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// Error handling for MetricsSessionEndAsync errors
		}
	}

	// Request the SDK begins an async shutdown by invoking Modio::ShutdownAsync
	// Parameters:
	// Callback Callback invoked when ShutdownAsync has completed. It takes a single ErrorCode parameter
	// indicating whether an error occurred.
	Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	}

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// No need for explicit handling of an error from ShutdownAsync, the application is about to exit
	}
}
