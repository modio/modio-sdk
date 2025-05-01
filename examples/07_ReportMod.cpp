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

/// This example extends 01_Initialization.cpp by demonstrating the functionality for
/// reporting games, users, or mods. This functionality is often a platform requirement. Reporting any of these
/// is possible via the ReportContentAsync function, which submits your report based on the parameters you provide, and
/// stores data validation errors if any occurred so you can retrieve them to display to users.

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
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to initialize mod.io SDK: " << ec.message() << std::endl;
		}

		Example.ExecutionFinished.set_value();
	}

	static void OnReportSubmitted(Modio::ErrorCode ec)
	{
		Example.LastError = ec;

		if (ec)
		{
			std::cout << "Failed to submit report: " << ec.message() << std::endl;
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

	UserInput = ModioExample::RetrieveUserInput("Provide the ModID to report:");
	IntTransformer << UserInput;
	int ModIDInt = 0;
	IntTransformer >> ModIDInt;

	// Populate the report with some example parameters. The first argument determines what type of report is being
	// submitted - game, mod, or user
	Modio::ReportParams Params =
		Modio::ReportParams(Modio::ModID(ModIDInt), Modio::ReportType::DMCA, "This mod contains copyrighted content",
							"ReporterName", "Reporter@Email.com");
	Modio::ReportContentAsync(Params, ModioExample::OnReportSubmitted);

	ModioExample::WaitForExecution();

	if (Example.LastError)
	{
		std::cout << "Mod report operation did not succeed" << std::endl;
	}
	else
	{
		std::cout << "Mod report succeed" << std::endl;
	}

	Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

	ModioExample::WaitForExecution();

	// Halt execution of Modio::RunPendingHandlers(), and wait for the background thread to finish
	bHaltBackgroundThread.store(true);
	if (HandlerThread.joinable())
	{
		HandlerThread.join();
	}
}
