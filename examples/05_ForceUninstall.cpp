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

/// This example extends 04_SubscriptionManagement.cpp by demonstrating the process for freeing up space in the mod
/// installation directory by force uninstalling a mod. In order to force uninstall a mod, the current user must not
/// have a subscription to it. Unsubscribing to a mod will remove it from the system if you were the only local user
/// with a subscription to it - Force uninstallation is for removing a mod that was installed by another user so that
/// you have space to install mods without having to switch to that other user.

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

	Modio::InitializeAsync(Modio::InitializeOptions(Modio::GameID(GameIDInt), Modio::ApiKey(APIStr), ModioEnv,
													Modio::Portal::None, SessionID),
						   ModioExample::OnInitializeComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		return -1;
	}

	// Check if the user has already been authenticated and the auth token is still valid
	Modio::VerifyUserAuthenticationAsync(ModioExample::OnVerifyUserAuthenticationAsync);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

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

				// Wait for async shutdown to complete
				while (!ModioExample::HasAsyncOperationCompleted())
				{
					Modio::RunPendingHandlers();
				};

				// Bail without running any other calls
				return -1;
			}
		}
		else
		{
			// Prompt user for input here
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

	// Enabling mod management requires an authenticated user, and explicitly permits the SDK to add or remove mods from
	// the system based on the user's subscriptions. If the SDK's local cache of user subscriptions indicates that the
	// user has a subscription to a mod which is not currently installed, it will begin to install that mod. Similarly,
	// if there is a mod installed on the system but no local users have a subscription to it, it will be uninstalled.
	// This means that if you have an open file handle on a file from a mod, but the user unsubscribes to it and no
	// other local accounts have a subscription to it, if Mod Management is enabled then the SDK will attempt to delete
	// that file.
	// As a result, in most circumstances you will want Mod Management to be disabled during active gameplay while mod
	// files are being opened by your game.
	// EnableModManagement requires you provide it with a callback - *unlike the callbacks provided to Async methods,
	// this callback will be invoked any time a mod management event is emitted by the SDK until Mod
	// Management is disabled, which means it will potentially be invoked multiple times.*
	// Mod management MUST be enabled in order for calls to Subscribe or Unsubscribe to succeed.
	// Parameters:
	// Callback Callback invoked for each management event
	// Returns a Modio::ErrorCode indicating if mod management was enabled successfully
	if (Modio::ErrorCode ec = Modio::EnableModManagement(ModioExample::OnModManagementEvent))
	{
		// An error occurred, bail early
		std::cout << "Could not enable mod management: " << ec.message() << std::endl;
		return -1;
	}
    
    {
        // Request a list of the user's subscriptions from the server, and cache them locally
        // This updates the local view of the users subscriptions, including any that were added from another device, or a
        // web browser
        // This requires mod management to be enabled and will immediately enqueue any pending installs, updates or
        // uninstallations based on the new subscription list
        // Parameters:
        // Callback Callback invoked when fetching updates has completed. Takes a Modio::ErrorCode indicating if the fetch
        // was successful
        Modio::FetchExternalUpdatesAsync(ModioExample::OnFetchExternalUpdatesComplete);

        while (!ModioExample::HasAsyncOperationCompleted())
        {
            Modio::RunPendingHandlers();
        };

        if (!ModioExample::DidAsyncOperationSucceed())
        {
            // No need for explicit handling of an error from FetchExternalUpdates in this simple sample
        }
    }
	
    // Using QuerySystemInstallations here to get all mods installed, regardless of whether they are in the current
    // user's subscriptions or not
    std::map<Modio::ModID, Modio::ModCollectionEntry> InstalledMods = Modio::QuerySystemInstallations();
    
    if (InstalledMods.size() <= 0) // Continue when no mods are installed
    {
        std::cout << "This example requires multiple mods installed to force an uninstall. Finishing execution." << std::endl;
    }
    else // Prompt the user for a Mod ID to uninstall
	{
		for (const auto& Mod : InstalledMods)
		{
			std::cout << Mod.first << "\t" << Mod.second.GetModProfile().ProfileName << "\t"
					  << Mod.second.GetSizeOnDisk().value_or(Modio::FileSize(0)) << std::endl;
		}
		std::int64_t UserModID = -1;
		std::string UserModString = ModioExample::RetrieveUserInput("Enter the ID of the mod you wish to force remove:");
		
		try
		{
			UserModID = std::stoll(UserModString);
		}
		catch (const std::exception&)
		{
			std::cout << "Invalid Input" << std::endl;
			return -1;
		}
        
        auto IsModContained = InstalledMods.find(Modio::ModID(UserModID));
        
        if (UserModID != -1 &&
            IsModContained != InstalledMods.end())
		{
			std::cout << "Force Uninstalling Mod ID:" << UserModID << std::endl;
			
            /// Forcibly uninstalls a mod from the system. This is intended for use when a host application requires more
            /// room for a mod that the user wants to install. It returns an error if the current user is
            /// subscribed to the mod. To remove a mod the current user is subscribed to, use "UnsubscribeFromModAsync"
            /// @param ModToRemove The ID for the mod to force remove.
            /// @param Callback Callback invoked when the uninstallation is successful, or if it failed because the current user
            /// remains subscribed.
			Modio::ForceUninstallModAsync(Modio::ModID(UserModID), ModioExample::OnUninstallationComplete);

			while (!ModioExample::HasAsyncOperationCompleted())
			{
				Modio::RunPendingHandlers();
			};

			if (!ModioExample::DidAsyncOperationSucceed())
			{
				// Explicit handling of an error from ForceUninstallModAsync in this simple sample
                // Most likely "ForceUninstallModAsync" could not execute if the user is subscribed to the mod
                std::cout << "Consider authentication with another user if the operation failed." << std::endl;
			}
			else
			{
				// Wait for the mod management system to finish removing the mod
                while (Example.LastEvent.Event != Modio::ModManagementEvent::EventType::Uninstalled)
				{
					Modio::RunPendingHandlers();
				}
			}
		}
        else
        {
            std::cout << "Invalid ModID registered, closing execution now" << std::endl;
        }
	}

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
