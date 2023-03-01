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

/// This example extends 01_Initialization.cpp by demonstrating querying and filtering
/// available mods for a game. It begins by performing an asynchronous initialization of the SDK, and if that is
/// successful will demonstrate a number of queries/filters on the available mods:
/// * Default query
/// * Result count limits
/// * Pagination
/// * Mod name search
/// * Mod sorting
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

	/// @brief Variable storing the most recent set of mods fetched by a call to ListAllModsAsync. Set to empty if
	/// ListAllModsAsync fails
	Modio::Optional<Modio::ModInfoList> LastResults;
};

/// @brief Static definition of the ModioExampleFlags. Consider a more sophisticated process
/// to deal with asynchronous calls should be designed in your game
static ModioExampleFlags Example;

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
	}

	static void NotifyApplicationFailure(Modio::ErrorCode ec)
	{
		Example.bDone = true;
		Example.bSuccess = false;

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

	/// @brief User callback invoked by Modio::RunPendingHandlers() when Modio::ListAllModsAsync has completed
	/// @param ec Error code indicating the success or failure of ShutdownAsync
	/// @param ModList Optional ModInfoList containing the results if the call was successful, or an empty optional if
	/// not
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

	static void ShutdownModIOSDK()
	{
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

	/// Default filtering
	{
		std::cout << "Default filtering:" << std::endl;
		// Request the SDK fetch the available mods for the current game
		// Parameters:
		// Filter The filter parameters to apply. Empty filter has the following default values:
		// * Sort by mod ID
		// * Sort in ascending order
		// * Result count 100 (show at most 100 results, the maximum supported by the REST API)
		// Callback Callback invoked when ListAllModsComplete has completed. It takes an ErrorCode parameter indicating
		// whether an error occurred, and a Modio::Optional<ModInfoList> containing the results if the call was
		// successful.
		Modio::ListAllModsAsync({}, ModioExample::OnListAllModsComplete);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		};

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// The call to ListAllModsAsync completed but returned an error
			std::cout << "Call failed, mod.io SDK shut down" << std::endl;

			// Always finalize the SDK.
			ModioExample::ShutdownModIOSDK();

			// Terminate execution
			exit(-1);
		}
		else
		{
			// The call to ListAllModsAsync completed successfully, so the SDK guarantees that Example.LastResults will
			// have a value. Performing the check here anyway to demonstrate how to check if a Modio::Optional is empty
			if (Example.LastResults.has_value())
			{
				// Example.LastResults is guaranteed to have a valid ModInfoList, however if a search returned no
				// results then GetResultCount() will still be 0
				std::cout << "Result count:" << Example.LastResults->GetResultCount() << std::endl;
				// Retrieve the value from a Modio::Optional by either dereferencing it or calling `value`
				// ModInfoList objects support iteration with range-based for
				for (const Modio::ModInfo& CurrentModInfo : Example.LastResults.value())
				{
					// Print a tab-delimited table containing Mod ID, Mod Name, and total downloads
					std::cout << CurrentModInfo.ModId << "\t" << CurrentModInfo.ProfileName << "\t"
							  << CurrentModInfo.Stats.DownloadsTotal << std::endl;
				}
			}
		}
	}

	/// Indexed results
	{
		std::cout << "Indexed results:" << std::endl;
		// As per default filtering, but asking for indexed results, starting at index 0 and asking for up to 3 results
		// returned.
		Modio::ListAllModsAsync(Modio::FilterParams().IndexedResults(0, 3), ModioExample::OnListAllModsComplete);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		};

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// The call to ListAllModsAsync completed but returned an error
			std::cout << "Call failed" << std::endl;

			// Always finalize the SDK.
			ModioExample::ShutdownModIOSDK();

			// Terminate execution
			exit(-2);
		}
		else
		{
			// The call to ListAllModsAsync completed successfully, so the SDK guarantees that Example.LastResults will
			// have a value. Performing the check here anyway to demonstrate how to check if a Modio::Optional is empty
			if (Example.LastResults.has_value())
			{
				// Example.LastResults is guaranteed to have a valid ModInfoList, however if a search returned no
				// results then GetResultCount() will still be 0
				std::cout << "Result count:" << Example.LastResults->GetResultCount() << std::endl;
				// Retrieve the value from a Modio::Optional by either dereferencing it or calling `value`
				// ModInfoList objects support iteration with range-based for
				for (const Modio::ModInfo& CurrentModInfo : Example.LastResults.value())
				{
					// Print a tab-delimited table containing Mod ID, Mod Name, and total downloads
					std::cout << CurrentModInfo.ModId << "\t" << CurrentModInfo.ProfileName << "\t"
							  << CurrentModInfo.Stats.DownloadsTotal << std::endl;
				}
			}
		}
	}

	/// Paginated results
	{
		std::cout << "Paginated results:" << std::endl;
		// As per default filtering, but asking for paginated results - this time requesting the page index 1 (second
		// page) containing 4 results This is equivalent to asking for IndexedResults(4, 4) but a little more convenient
		// when you have a paginated UI to display the results in
		Modio::ListAllModsAsync(Modio::FilterParams().PagedResults(1, 4), ModioExample::OnListAllModsComplete);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		};

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// The call to ListAllModsAsync completed but returned an error
			std::cout << "Call failed" << std::endl;

			// Always finalize the SDK.
			ModioExample::ShutdownModIOSDK();

			// Terminate execution
			exit(-3);
		}
		else
		{
			// The call to ListAllModsAsync completed successfully, so the SDK guarantees that Example.LastResults will
			// have a value. Performing the check here anyway to demonstrate how to check if a Modio::Optional is empty
			if (Example.LastResults.has_value())
			{
				// Example.LastResults is guaranteed to have a valid ModInfoList, however if a search returned no
				// results then GetResultCount() will still be 0
				std::cout << "Result count:" << Example.LastResults->GetResultCount() << std::endl;
				// Retrieve the value from a Modio::Optional by either dereferencing it or calling `value`
				// ModInfoList objects support iteration with range-based for
				for (const Modio::ModInfo& CurrentModInfo : Example.LastResults.value())
				{
					// Print a tab-delimited table containing Mod ID, Mod Name, and total downloads
					std::cout << CurrentModInfo.ModId << "\t" << CurrentModInfo.ProfileName << "\t"
							  << CurrentModInfo.Stats.DownloadsTotal << std::endl;
				}
			}
		}
	}

	/// Name searching
	{
		std::cout << "Searching for strings in mod names:" << std::endl;
		// Filter parameters can be combined, with the last call to a parameter-setting function overriding previous
		// calls. This combination of calls asks for the first 10 results using the word "Sample" in the mod name,
		// leaving all other parameters at default values.
		Modio::ListAllModsAsync(Modio::FilterParams().IndexedResults(0, 10).NameContains("Sample"),
								ModioExample::OnListAllModsComplete);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		};

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// The call to ListAllModsAsync completed but returned an error
			std::cout << "Call failed" << std::endl;

			// Always finalize the SDK.
			ModioExample::ShutdownModIOSDK();

			// Terminate execution
			exit(-4);
		}
		else
		{
			// The call to ListAllModsAsync completed successfully, so the SDK guarantees that Example.LastResults will
			// have a value. Performing the check here anyway to demonstrate how to check if a Modio::Optional is empty
			if (Example.LastResults.has_value())
			{
				// Example.LastResults is guaranteed to have a valid ModInfoList, however if a search returned no
				// results then GetResultCount() will still be 0
				std::cout << "Result count:" << Example.LastResults->GetResultCount() << std::endl;
				// Retrieve the value from a Modio::Optional by either dereferencing it or calling `value`
				// ModInfoList objects support iteration with range-based for
				for (const Modio::ModInfo& CurrentModInfo : Example.LastResults.value())
				{
					// Print a tab-delimited table containing Mod ID, Mod Name, and total downloads
					std::cout << CurrentModInfo.ModId << "\t" << CurrentModInfo.ProfileName << "\t"
							  << CurrentModInfo.Stats.DownloadsTotal << std::endl;
				}
			}
		}
	}

	/// Sorting results
	{
		std::cout << "Sorted results:" << std::endl;
		// This example retains the existing settings regarding result count limits and name searching, but requests the
		// results be sorted by descending rating rather than ascending mod ID.
		Modio::FilterParams SortParameters = Modio::FilterParams().IndexedResults(0, 10).NameContains("Sample");
		// Additional parameters can be added after filter construction.
		SortParameters.SortBy(Modio::FilterParams::SortFieldType::Rating,
							  Modio::FilterParams::SortDirection::Descending);
		Modio::ListAllModsAsync(SortParameters, ModioExample::OnListAllModsComplete);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		};

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// The call to ListAllModsAsync completed but returned an error
			std::cout << "Call failed" << std::endl;

			// Always finalize the SDK.
			ModioExample::ShutdownModIOSDK();

			// Terminate execution
			exit(-5);
		}
		else
		{
			// The call to ListAllModsAsync completed successfully, so the SDK guarantees that Example.LastResults will
			// have a value. Performing the check here anyway to demonstrate how to check if a Modio::Optional is empty
			if (Example.LastResults.has_value())
			{
				// Example.LastResults is guaranteed to have a valid ModInfoList, however if a search returned no
				// results then GetResultCount() will still be 0
				std::cout << "Result count:" << Example.LastResults->GetResultCount() << std::endl;
				// Retrieve the value from a Modio::Optional by either dereferencing it or calling `value`
				// ModInfoList objects support iteration with range-based for
				for (const Modio::ModInfo& CurrentModInfo : Example.LastResults.value())
				{
					// Print a tab-delimited table containing Mod ID, Mod Name, and total downloads
					std::cout << CurrentModInfo.ModId << "\t" << CurrentModInfo.ProfileName << "\t"
							  << CurrentModInfo.Stats.DownloadsTotal << std::endl;
				}
			}
		}
	}

	// Always finalize the SDK.
	ModioExample::ShutdownModIOSDK();

	return 0;
}
