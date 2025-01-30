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

/// This demonstrates the mod.io SDK's Temporary Mod Set feature.
/// Temp Mod Sets are intended to be used as transient installations, where a mod can exist in
///	local temp storage and be loaded for a single gameplay session without requiring a user
///	to subscribe to it. This is good for things such as multiplayer scenarios, where
///	you might want to replicate user-owned cosmetics from clients, or load
///	a single map without requiring subscription.
///	Temporary mods are cleared on SDK re-initialization, so storage is not persisted across sessions.

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
		std::cout << ec.message() << std::endl;
	}

	static void OnInitializeComplete(Modio::ErrorCode ec)
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

	static void OnRequestEmailAuthCodeCompleted(Modio::ErrorCode ec)
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

	static void OnAuthenticateUserEmailCompleted(Modio::ErrorCode ec)
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

	static void OnVerifyUserAuthenticationAsync(Modio::ErrorCode ec)
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

	/// @brief User Callback invoked by the SDK when a mod is installed, updated, uploaded or removed from the local
	/// system
	/// @param ManagementEvent Event object containing information about what type of event occurred, the mod ID, and an
	/// error code if an error occurred
	static void OnModManagementEvent(Modio::ModManagementEvent ManagementEvent)
	{
		// Register the last event
		Example.LastEvent = ManagementEvent;

		// Inspect the type of event - Installed, Updated, Uploaded, or Uninstalled
		switch (ManagementEvent.Event)
		{
			case Modio::ModManagementEvent::EventType::Installed:
			{
				// Status will be truthy if it contains an error
				if (ManagementEvent.Status)
				{
					// Retrieve a developer-facing error message by invoking `message` on the error code
					std::cout << "Mod Management event: Mod ID " << ManagementEvent.ID
							  << " failed to install with error :" << ManagementEvent.Status.message() << std::endl;
				}
				else
				{
					std::cout << "Mod Management event: Mod ID " << ManagementEvent.ID << " install succeeded!"
							  << std::endl;
				}
				break;
			}
			case Modio::ModManagementEvent::EventType::Uninstalled:
			{
				// Status will be truthy if it contains an error
				if (ManagementEvent.Status)
				{
					// Retrieve a developer-facing error message by invoking `message` on the error code
					std::cout << "Mod Management event: Mod ID " << ManagementEvent.ID
							  << " failed to uninstall with error :" << ManagementEvent.Status.message() << std::endl;
				}
				else
				{
					std::cout << "Mod Management event: Mod ID " << ManagementEvent.ID << " removal succeeded!"
							  << std::endl;
				}
				break;
			}
			default:
				break;
		}
	}

	static void OnFetchExternalUpdatesComplete(Modio::ErrorCode ec)
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
	static void OnSubscriptionComplete(Modio::ErrorCode ec)
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
	static void OnUninstallationComplete(Modio::ErrorCode ec)
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

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	}

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		return -1;
	}

	// Check if the user has already been authenticated and the auth token is still valid
	Modio::VerifyUserAuthenticationAsync(ModioExample::OnVerifyUserAuthenticationAsync);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	}

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// There is no user or the auth token is not valid anymore, therefore it should authenticate again
		{
			std::string UserEmailAddress = ModioExample::RetrieveUserInput("Enter email address:");

			Modio::RequestEmailAuthCodeAsync(Modio::EmailAddress(UserEmailAddress),
											 ModioExample::OnRequestEmailAuthCodeCompleted);
		}

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		};

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			if (!Modio::ErrorCodeMatches(Example.LastError, Modio::ErrorConditionTypes::UserAlreadyAuthenticatedError))
			{
				Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

				while (!ModioExample::HasAsyncOperationCompleted())
				{
					Modio::RunPendingHandlers();
				};

				return -1;
			}
		}
		else
		{
			std::string UserEmailAuthCode = ModioExample::RetrieveUserInput("Enter email auth code:");

			Modio::AuthenticateUserEmailAsync(Modio::EmailAuthCode(UserEmailAuthCode),
											  ModioExample::OnAuthenticateUserEmailCompleted);
			while (!ModioExample::HasAsyncOperationCompleted())
			{
				Modio::RunPendingHandlers();
			};

			if (!ModioExample::DidAsyncOperationSucceed()) {}
		}
	}

	// Display a list of available mods so the user can select one to install
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

	// Enabling mod management requires an authenticated user, and explicitly permits the SDK to add or remove mods from
	// the system based on the user's subscriptions. If the SDK's local cache of user subscriptions indicates that the
	// user has a subscription to a mod which is not currently installed, it will begin to install that mod. Similarly,
	// if there is a mod installed on the system but no local users have a subscription to it, it will be uninstalled.
	// This means that if you have an open file handle on a file from a mod, but the user unsubscribes to it and no
	// other local accounts have a subscription to it, if Mod Management is enabled then the SDK will attempt to delete
	// that file.
	//
	// As a result, in most circumstances you will want Mod Management to be disabled during active gameplay while mod
	// files are being opened by your game.
	//
	// EnableModManagement requires you provide it with a callback. Unlike the callbacks provided to async methods,
	// this callback will be invoked any time a mod management event is emitted by the SDK until Mod
	// Management is disabled. It will likely be invoked multiple times for multiple events.
	//
	// Mod management MUST be enabled for calls to Subscribe or Unsubscribe to succeed.
	//
	// Parameters: Callback - Callback invoked for each management event. Returns a Modio::ErrorCode indicating if mod
	// management was enabled successfully
	if (Modio::ErrorCode ec = Modio::EnableModManagement(ModioExample::OnModManagementEvent))
	{
		// An error occurred, bail early
		std::cout << "Could not enable mod management: " << ec.message() << std::endl;
		return -1;
	}

	std::cout << "Checking for subscription updates from the mod.io service.." << std::endl;

	{
		// Request a list of the user's subscriptions from the server and cache them locally
		// This updates the local view of the users subscriptions, including any that were added from another device or
		// a web browser. This requires mod management to be enabled and will immediately enqueue any pending installs,
		// updates or uninstallations based on the new subscription list.
		//
		// Parameters: Callback - Callback invoked when fetching updates has completed. Takes a Modio::ErrorCode
		// indicating if the fetch was successful
		Modio::FetchExternalUpdatesAsync(ModioExample::OnFetchExternalUpdatesComplete);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		}

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// No need for explicit handling of an error from FetchExternalUpdates in this simple sample
		}
	}

	// Prompt the user for a Mod ID to install
	{
		std::int64_t UserModID = -1;
		// An example ModID: "2281875", named "Sherlock Holmes"
		std::string UserModString = ModioExample::RetrieveUserInput("Enter the ID of the mod you wish to install:");

		UserModID = std::strtoll(UserModString.c_str(), nullptr,10);
		if (UserModID <= 0)
		{
			std::cout << "Invalid Input" << std::endl;
			return -1;
		}
		std::cout << "Initializing a Temporary Mod Set with Mod ID: " << UserModID << std::endl;
		// Initialize a Temporary Mod Set with a given set of Mod IDs.
		// Once the set has been created, you can add or remove mods to it later
		std::vector<Modio::ModID> TempModList;
		TempModList.emplace_back(UserModID);

		(void) Modio::InitTempModSet(TempModList);

		// Note that the TempModSet functions are not Async, meaning you get an immediate return.
		// You will, however receive a Mod Management event once installation of the Temp Mod is successful
		// Wait for the mod management system to finish installing the new mod and anything else which is
		// pending
		while (Modio::IsModManagementBusy())
		{
			Modio::RunPendingHandlers();
		}
	}

	// Prompt the user for a Mod ID to add to the Temp Mod Set
	{
		std::int64_t UserModID = -1;
		std::string UserModString =
			ModioExample::RetrieveUserInput("Enter the ID of the mod you wish to add to the Temp Mod Set:");

		if (UserModID < 0)
		{
			std::cout << "Invalid Input" << std::endl;
			return -1;
		}

		std::cout << "Adding Mod ID to Temp Mod Set: " << UserModID << std::endl;

		// After you have initialized a TempModSet, you can add additional mods to it - for instance if a user
		// changes their cosmetic attachments, or if a new player joins the game
		std::vector<Modio::ModID> TempModList;
		TempModList.emplace_back(Modio::ModID(UserModID));
		Modio::ErrorCode ec = Modio::AddToTempModSet(TempModList);
		(void) ec;

		// Once again, the TempModSet functions are not async, but will trigger a ModManagement event, so
		// we should wait for mod management to install this mod.
		while (Modio::IsModManagementBusy())
		{
			Modio::RunPendingHandlers();
		}
	}

	// Query the Temp Mod Set
	{
		// Once you have installed Temp Mods, you can query their installation location using QueryTempModSet.
		for (auto Mod : Modio::QueryTempModSet())
		{
			std::cout << "Mod ID " << Mod.first << " is installed to location " << Mod.second.GetPath() << std::endl;
		}
	}

	// Close the TempModSet
	{
		// Once you have finished a session, you should call CloseTempModSet. This will handle cleanup of the data
		// and close the set, requiring a new one to be initialized next time.
		// If you do not do this, temp mods will be auto cleaned up next time the SDK is initialized

		(void) Modio::CloseTempModSet();
	}

	Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	if (!ModioExample::DidAsyncOperationSucceed()) {}
}
