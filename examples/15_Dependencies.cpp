/*
 *  Copyright (C) 2020-2025 mod.io Pty Ltd. <https://mod.io>
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

 /// This example shows how to add, remove and list all dependencies for a mod

 /// @brief Simple struct containing state variables for the example
struct ModioExampleFlags
{
	/// @brief Promise to halt execution of the main thread til this is fulfilled
	std::promise<void> ExecutionFinished;

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
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to initialize mod.io SDK: " << ec.message() << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	static void OnRequestEmailAuthCodeCompleted(Modio::ErrorCode ec)
	{
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to request email auth code: " << ec.message() << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	static void OnAuthenticateUserEmailCompleted(Modio::ErrorCode ec)
	{
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

	static void OnVerifyUserAuthenticationAsync(Modio::ErrorCode ec)
	{
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to authorize user: " << ec.message() << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	static void OnGetModDependenciesCompleted(Modio::ErrorCode ec, Modio::Optional<Modio::ModDependencyList> Deps)
	{
		if (ec)
		{
			std::cout << "Failed to get mod dependencies: " << ec.message() << std::endl;
		}
		else
		{
			if (Deps.has_value() && Deps.value().Size() > 0)
			{
				std::cout << "Dependencies: " << std::endl;
				for (auto& Mod : Deps.value())
				{
					std::cout << Mod.ModName << std::endl;
				}
			}
			else
			{
				std::cout << "No dependencies!" << std::endl;
			}
		}
		Example.ExecutionFinished.set_value();
	}

	static void OnAddModDependencyCompleted(Modio::ErrorCode ec)
	{
		if (ec)
		{
			std::cout << "Failed to add dependencies: " << ec.message() << std::endl;
		}
		else
		{
			std::cout << "Added dependencies." << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	static void OnDeleteModDependencyCompleted(Modio::ErrorCode ec)
	{
		if (ec)
		{
			std::cout << "Failed to delete dependencies: " << ec.message() << std::endl;
		}
		else
		{
			std::cout << "Deleted dependencies." << std::endl;
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
	// A thread-safe atomic boolean to control the execution of Modio::RunPendingHandlers() on the background thread
	std::atomic<bool> bHaltBackgroundThread{ false };

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
		// There is no user or the auth token is not valid anymore, therefore it should authenticate again
		{
			std::string UserEmailAddress = ModioExample::RetrieveUserInput("Enter email address:");

			Modio::RequestEmailAuthCodeAsync(Modio::EmailAddress(UserEmailAddress),
				ModioExample::OnRequestEmailAuthCodeCompleted);
		}

		ModioExample::WaitForExecution();

		if (Example.LastError)
		{
			if (!Modio::ErrorCodeMatches(Example.LastError, Modio::ErrorConditionTypes::UserAlreadyAuthenticatedError))
			{
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
		}
		else
		{
			std::string UserEmailAuthCode = ModioExample::RetrieveUserInput("Enter email auth code:");

			Modio::AuthenticateUserEmailAsync(Modio::EmailAuthCode(UserEmailAuthCode),
				ModioExample::OnAuthenticateUserEmailCompleted);

			ModioExample::WaitForExecution();
		}
	}

	// The mod with which to add and remove a dependency
	int64_t Parent = stol(ModioExample::RetrieveUserInput("Enter the Parent Mod ID: ", "-1"));

	// This is the mod to add, then remove from the parent mod
	int64_t Child = stol(ModioExample::RetrieveUserInput("Enter the Child Mod ID: ", "-1"));
	Modio::ModID ParentModID(Parent);
	Modio::ModID ChildModID(Child);

	std::vector<Modio::ModID> DependenciesToAdd;
	DependenciesToAdd.push_back(ChildModID);

	Modio::AddModDependenciesAsync(ParentModID, DependenciesToAdd, ModioExample::OnAddModDependencyCompleted);

	ModioExample::WaitForExecution();

	Modio::GetModDependenciesAsync(ParentModID, true, ModioExample::OnGetModDependenciesCompleted);

	ModioExample::WaitForExecution();

	Modio::DeleteModDependenciesAsync(ParentModID, DependenciesToAdd, ModioExample::OnDeleteModDependencyCompleted);

	ModioExample::WaitForExecution();

	Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

	ModioExample::WaitForExecution();

	// Halt execution of Modio::RunPendingHandlers(), and wait for the background thread to finish
	bHaltBackgroundThread.store(true);
	if (HandlerThread.joinable())
	{
		HandlerThread.join();
	}
}
