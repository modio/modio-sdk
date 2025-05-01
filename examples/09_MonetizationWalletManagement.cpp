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

#include <atomic>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <utility>

/// This example demonstrates usage of the mod.io wallet management features, including
/// getting the user's wallet balance.
/// Once these queries have been performed, the sample initiates an async shutdown of the SDK if necessary and then
/// terminates.

/// @brief Simple struct containing state variables for the example
struct ModioExampleFlags
{
	/// @brief Promise to halt execution of the main thread til this is fulfilled
	std::promise<void> ExecutionFinished;

	/// @brief Keeps a reference to the last error received
	Modio::ErrorCode LastError;
};

/// @brief Static definition of the ModioExampleFlags. Consider a more sophisticated process
/// to deal with asynchronous calls should be designed in your game
static ModioExampleFlags Example;

/// @brief Current balance of the users wallet
static uint64_t UserWalletBalance;

/// @brief Static definition of the ModioExampleFlags. Consider a more sophisticated process
/// to deal with asynchronous calls should be designed in your game
struct ModioExample
{
	/// @brief Waits for the Example.ExecutionFinished promise to be fulfilled, then resets it
	static void WaitForExecution()
	{
		Example.ExecutionFinished.get_future().wait();
		Example.ExecutionFinished = std::promise<void>();
	}

	static void OnInitializeComplete(Modio::ErrorCode ec)
	{
		// The ErrorCode value passed back via the callback will be implicitly convertible to
		// true if an error occurred
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to initialize mod.io SDK: " << ec.message() << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	static void OnVerifyUserAuthenticationAsync(Modio::ErrorCode ec)
	{
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to authorize user: " << ec.message() << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	/// @brief User callback invoked by Modio::RunPendingHandlers() when Modio::RequestEmailAuthCodeAsync has completed
	/// @param ec Error code indicating the success or failure of RequestEmailAuthCodeAsync
	static void OnRequestEmailAuthCodeCompleted(Modio::ErrorCode ec)
	{
		// The ErrorCode value passed back via the callback will be implicitly convertible to
		// true if an error occurred
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to request email auth code: " << ec.message() << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	/// @brief User callback invoked by Modio::RunPendingHandlers() when Modio::AuthenticateUserEmailAsync has completed
	/// @param ec Error code indicating the success or failure of AuthenticateUserEmailAsync
	static void OnAuthenticateUserEmailCompleted(Modio::ErrorCode ec)
	{
		// The ErrorCode value passed back via the callback will be implicitly convertible to
		// true if an error occurred
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to authenticate user email: " << ec.message() << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	static void OnShutdownComplete(Modio::ErrorCode ec)
	{
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to shutdown: " << ec.message() << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	static void OnGetWalletBalanceCompleted(Modio::ErrorCode ec, Modio::Optional<uint64_t> WalletBalance)
	{
		Example.LastError = ec;
		if (ec)
		{
			std::cout << "Failed to get wallet balance: " << ec.message() << std::endl;
		}
		else
		{
			UserWalletBalance = WalletBalance.value();
			std::cout << "Users wallet balance is: " << std::to_string(UserWalletBalance) << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	static void OnRefreshUserEntitlementsCompleted(
		Modio::ErrorCode ec, Modio::Optional<Modio::EntitlementConsumptionStatusList> EntitlementConsumptionStatus)
	{
		Example.LastError = ec;
		if (ec)
		{
			std::cout << "Failed to refresh entitlements: " << ec.message() << std::endl;
		}
		else
		{
			// Update the users wallet balance from the refresh call if any adjustments have been made.
			// Note that if nothing is consumed, then WalletBalance may not exist or be 0.
			if (EntitlementConsumptionStatus->WalletBalance.has_value())
			{
				UserWalletBalance = EntitlementConsumptionStatus.value().WalletBalance.value().Balance;
			}

			std::cout << "Updated wallet balance is: " << std::to_string(UserWalletBalance) << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	/// @brief Helper method to retrieve input from a user with an string prompt
	/// @param Prompt The string to show related to the expected user input
	/// @param DefaultValue If the user wants to use a default input, this will be used
	/// @return String with the user input or default value
	static std::string RetrieveUserInput(std::string Prompt, std::string DefaultValue = std::string())
	{
		std::string UserInput;
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
	// A thread-safe atomic boolean to control the execution of Modio::RunPendingHandlers() on the background thread
	std::atomic<bool> bHaltBackgroundThread {false};

	// Begin new thread for background work
	std::thread HandlerThread = std::thread([&]() {
		while (!bHaltBackgroundThread)
		{
			Modio::RunPendingHandlers();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	});

	// Ask user for their input and offer default values
	std::stringstream IntTransformer;
	std::string UserInput = ModioExample::RetrieveUserInput("Game ID:", "3609");
	IntTransformer << UserInput;
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

	Modio::InitializeAsync(Modio::InitializeOptions(Modio::GameID(GameIDInt), Modio::ApiKey(APIStr), ModioEnv,
													Modio::Portal::None, SessionID),
						   ModioExample::OnInitializeComplete);

	ModioExample::WaitForExecution();

	if (Example.LastError)
	{
		return -1;
	}

	// Check if the user has already been authenticated and the auth token is still valid
	Modio::VerifyUserAuthenticationAsync(ModioExample::OnVerifyUserAuthenticationAsync);

	ModioExample::WaitForExecution();

	if (Example.LastError)
	{
		{
			// Prompt user for input here
			std::string UserEmailAddress = ModioExample::RetrieveUserInput("Enter email address:");

			// Email authentication with mod.io is a two-step process. First, a user email address is submitted via
			// RequestEmailAuthCodeAsync, which triggers an email containing an authentication code to that address.
			Modio::RequestEmailAuthCodeAsync(Modio::EmailAddress(UserEmailAddress),
											 ModioExample::OnRequestEmailAuthCodeCompleted);
		}

		ModioExample::WaitForExecution();

		if (Example.LastError)
		{
			// It is possible to check the error type and identify what next steps to follow
			if (Modio::ErrorCodeMatches(Example.LastError,
										Modio::ErrorConditionTypes::NetworkError)) // NetworkError group
			{
				// Error code represents some network error kind. Possibly ask the user to try again later.
				std::cout << "A network error" << std::endl;
			}
			else if (Modio::ErrorCodeMatches(Example.LastError,
											 Modio::ErrorConditionTypes::EntityNotFoundError)) // Entity Not Found group
			{
				// An mod entity is not located with this configuration. Therefore, the list you're fetching the ModID
				// from is probably stale. A remedy could be to fetch an updated version of the list from the server.
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

			ModioExample::WaitForExecution();

			// Halt execution of Modio::RunPendingHandlers(), and wait for the background thread to finish
			bHaltBackgroundThread.store(true);
			if (HandlerThread.joinable())
			{
				HandlerThread.join();
			}

			return -1;
		}

		{
			// Prompt user for input here
			std::string UserEmailAuthCode = ModioExample::RetrieveUserInput("Enter email auth code:");

			// The second part of email authentication with the mod.io SDK involves submitting the user's email
			// authentication code. To do this, you will need to prompt your user for input and then pass that input to
			// AuthenticateUserEmailAsync. If this function returns successfully, the mod.io account will be stored in
			// the Local User Session passed into InitializeAsync earlier.
			Modio::AuthenticateUserEmailAsync(Modio::EmailAuthCode(UserEmailAuthCode),
											  ModioExample::OnAuthenticateUserEmailCompleted);
		}

		ModioExample::WaitForExecution();

		if (Example.LastError)
		{
			// Perform any auth error handling here as appropriate.
		}
	}

	// Get the user's wallet balance for the current game. Note that if a wallet does not exist for a user, this call
	// will automatically create the wallet for them.
	{
		Modio::GetUserWalletBalanceAsync(ModioExample::OnGetWalletBalanceCompleted);

		ModioExample::WaitForExecution();

		if (Example.LastError)
		{
			// Error handling for GetWalletBalance errors
		}
	}

	// Request the SDK begins an async shutdown by invoking Modio::ShutdownAsync
	// Parameters:
	// Callback Callback invoked when ShutdownAsync has completed. It takes a single ErrorCode parameter
	// indicating whether an error occurred.
	Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

	ModioExample::WaitForExecution();

	// Halt execution of Modio::RunPendingHandlers(), and wait for the background thread to finish
	bHaltBackgroundThread.store(true);
	if (HandlerThread.joinable())
	{
		HandlerThread.join();
	}
}
