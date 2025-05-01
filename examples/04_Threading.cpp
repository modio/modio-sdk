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
#include <thread>

/// This example demonstrates calling Modio::RunPendingHandlers() on a background thread. This can improve performance
/// by decoupling the application's main loop from the frequency with which SDK work is performed.
///
/// The function calls used in this example are similar to those used in 02_ModQueries.

/// @brief Helper method to print information from a Modio::ModInfoList to the console
static void PrintModInfoList(Modio::ModInfoList& List)
{
	std::cout << "Result count: " << List.GetResultCount() << std::endl;

	for (const Modio::ModInfo& Mod : List)
	{
		std::cout << Mod.ModId << "\t" << Mod.ProfileName << "\t" << Mod.Stats.DownloadsTotal << "\n";
	}
	std::cout << std::endl;
}

/// @brief Helper method to retrieve input from a user with an string prompt
static std::string RetrieveUserInput(std::string Prompt, std::string DefaultValue = std::string())
{
	std::string UserInput;
	std::cout << Prompt << std::endl;

	if (DefaultValue.size() != 0)
	{
		std::cout << "(Tap enter to use the default value: " << DefaultValue << ")" << std::endl;
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
	std::string UserInput = RetrieveUserInput("Game ID:", "3609");
	IntTransformer << UserInput;
	std::string APIStr = RetrieveUserInput("API key:", "ca842a1f60c40bc8fb2044bc9932d763");
	std::string ModioEnvStr = RetrieveUserInput("Modio Environment:", "Live");
	std::string SessionID = RetrieveUserInput("SessionID:", "ExampleSession");

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

	// std::promise will be used to signal completion of an asynchronous function
	std::promise<void> InitializeComplete;

	Modio::InitializeAsync(Modio::InitializeOptions(Modio::GameID(GameIDInt), Modio::ApiKey(APIStr), ModioEnv,
													Modio::Portal::None, SessionID),
						   [&](Modio::ErrorCode ec) {
							   if (ec)
							   {
								   std::cout << "Failed to initialize mod.io SDK: " << ec.message() << std::endl;
							   }
							   InitializeComplete.set_value();
						   });

	// We need to wait for the SDK to be initialized before making subsequent calls
	InitializeComplete.get_future().wait();

	// Queue multiple Modio::ListAllModsAsync() calls. Upon completion, a callback will print the results (or the
	// relevant error message)

	// Default filtering
	std::promise<void> DefaultComplete;
	Modio::ListAllModsAsync({}, [&](Modio::ErrorCode ec, Modio::Optional<Modio::ModInfoList> InfoList) {
		std::cout << "Default filtering\n";
		if (ec)
		{
			std::cout << "Failure: " << ec.message() << std::endl;
		}
		else
		{
			PrintModInfoList(InfoList.value());
		}
		DefaultComplete.set_value();
	});

	// Indexed results
	// This example requests 2 entries, starting at index 0
	std::promise<void> IndexedComplete;
	Modio::ListAllModsAsync(Modio::FilterParams().IndexedResults(0, 2),
							[&](Modio::ErrorCode ec, Modio::Optional<Modio::ModInfoList> InfoList) {
								std::cout << "Indexed results\n";
								if (ec)
								{
									std::cout << "Failure: " << ec.message() << std::endl;
								}
								else
								{
									PrintModInfoList(InfoList.value());
								}
								IndexedComplete.set_value();
							});

	// Paginated results
	// This represents the second page (index 1) where each page contains 2 results
	std::promise<void> PaginatedComplete;
	Modio::ListAllModsAsync(Modio::FilterParams().PagedResults(1, 2),
							[&](Modio::ErrorCode ec, Modio::Optional<Modio::ModInfoList> InfoList) {
								std::cout << "Paginated results\n";
								if (ec)
								{
									std::cout << "Failure: " << ec.message() << std::endl;
								}
								else
								{
									PrintModInfoList(InfoList.value());
								}
								PaginatedComplete.set_value();
							});

	// Search by name
	std::promise<void> NameSearchComplete;
	Modio::ListAllModsAsync(Modio::FilterParams().IndexedResults(0, 10).NameContains("Dracula"),
							[&](Modio::ErrorCode ec, Modio::Optional<Modio::ModInfoList> InfoList) {
								std::cout << "Search by name\n";
								if (ec)
								{
									std::cout << "Failure: " << ec.message() << std::endl;
								}
								else
								{
									PrintModInfoList(InfoList.value());
								}
								NameSearchComplete.set_value();
							});

	// Sorted results

	// First, we're filtering by result count limits
	Modio::FilterParams SortParameters = Modio::FilterParams().IndexedResults(0, 10);

	// Additional parameters can be added after filter construction.
	// Here we request the results be sorted by total downloads (descending)
	SortParameters.SortBy(Modio::FilterParams::SortFieldType::DownloadsTotal,
						  Modio::FilterParams::SortDirection::Descending);

	std::promise<void> SortedComplete;
	Modio::ListAllModsAsync(SortParameters, [&](Modio::ErrorCode ec, Modio::Optional<Modio::ModInfoList> InfoList) {
		std::cout << "Sorted results\n";
		if (ec)
		{
			std::cout << "Failure: " << ec.message() << std::endl;
		}
		else
		{
			PrintModInfoList(InfoList.value());
		}
		SortedComplete.set_value();
	});

	// Other work can be performed here while Modio::ListAllModsAsync() is processed in the background

	// We can wait for Modio::ListAllModsAsync() calls to complete as required
	DefaultComplete.get_future().wait();
	IndexedComplete.get_future().wait();
	PaginatedComplete.get_future().wait();
	NameSearchComplete.get_future().wait();
	SortedComplete.get_future().wait();

	// Always finalize the SDK.
	std::promise<void> ShutdownComplete;
	Modio::ShutdownAsync([&](Modio::ErrorCode) { ShutdownComplete.set_value(); });
	ShutdownComplete.get_future().wait();

	// Halt execution of Modio::RunPendingHandlers(), and wait for the background thread to finish
	bHaltBackgroundThread.store(true);
	if (HandlerThread.joinable())
	{
		HandlerThread.join();
	}

	return 0;
}