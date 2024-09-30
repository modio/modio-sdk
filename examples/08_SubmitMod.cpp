/*
 *  Copyright (C) 2020-2024 mod.io Pty Ltd. <https://mod.io>
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

/// This example extends 03_Authentication.cpp by demonstrating how to submit new mod.
/// When Mod Management is enabled, the SDK will upload any mods that you submit.
/// The sample will submit a new mod to get a mod id,
/// The mod id will be used to submit a new mod file, the upload of the mod file will
/// be handled by the SDK's mod management loop enabled earlier.
/// For this test you will need to have a GameId and an API Key.
/// You will also need to provide a path to a logo and a folder which
/// will contain the mod data.


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
		std::cout << ec.message() << std::endl;
	}

	static void OnAsyncComplete(Modio::ErrorCode ec)
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
	IntTransformer << ModioExample::RetrieveUserInput("Game ID:");
	std::string APIStr = ModioExample::RetrieveUserInput("API key:");
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
						   ModioExample::OnAsyncComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	}

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		return -1;
	}


	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// There is no user or the auth token is not valid anymore, therefore it should authenticate again
		{
			std::string UserEmailAddress = ModioExample::RetrieveUserInput("Enter email address:");

			Modio::RequestEmailAuthCodeAsync(Modio::EmailAddress(UserEmailAddress),
											 ModioExample::OnAsyncComplete);
		}

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		}

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			if (!Modio::ErrorCodeMatches(Example.LastError, Modio::ErrorConditionTypes::UserAlreadyAuthenticatedError))
			{
				Modio::ShutdownAsync(ModioExample::OnAsyncComplete);

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
											  ModioExample::OnAsyncComplete);
			while (!ModioExample::HasAsyncOperationCompleted())
			{
				Modio::RunPendingHandlers();
			};

			if (!ModioExample::DidAsyncOperationSucceed()) {}
		}
	}
	
	bool UploadCompleted = false;

	// Enable mod management, providing a function to handle events for install/uninstall/upload
	// Enabling mod management requires you to be authenticated. Please refer to examples/03_Authentication.cpp for more information
	if (Modio::ErrorCode ec = Modio::EnableModManagement([&](Modio::ModManagementEvent ManagementEvent) {
			// See 04_Subscription_Management.cpp for more information on handling Mod Management Events
			switch (ManagementEvent.Event)
			{
					// Event firing on upload start
				case Modio::ModManagementEvent::EventType::BeginUpload:
				{
					// Status will be truthy if it contains an error
					if (ManagementEvent.Status)
					{
						// Retrieve a developer-facing error message by invoking `message` on the error code
						std::cout << "Mod Management event: Mod ID " << ManagementEvent.ID
								  << " encountered an error while beginning upload :"
								  << ManagementEvent.Status.message() << std::endl;
						// End early if there was an error starting the upload
						UploadCompleted = true;
					}
					else
					{
						std::cout << "Mod Management event: Mod ID " << ManagementEvent.ID << " upload starting"
								  << std::endl;
					}
					break;
				}
				// Event indicating upload completed
				case Modio::ModManagementEvent::EventType::Uploaded:
				{
					if (ManagementEvent.Status)
					{
						std::cout << "Mod Management event: Mod ID " << ManagementEvent.ID
								  << " encountered an error while uploading :" << ManagementEvent.Status.message()
								  << std::endl;
					}
					else
					{
						std::cout << "Mod Management event: Mod ID " << ManagementEvent.ID << " upload completed OK"
								  << std::endl;
					}
					UploadCompleted = true;
					break;
				}
				// Handle other mod management event types here
				default:
					break;
			}
		}))
	{
		// An error occurred, bail early
		std::cout << "Could not enable mod management: " << ec.message() << std::endl;
		return -1;
	}

	Modio::ModCreationHandle NewModHandle = Modio::GetModCreationHandle();
	Modio::CreateModParams NewModParams;
	// Mandatory fields for a new mod
	NewModParams.Name = "Sample Mod";
	NewModParams.PathToLogoFile = "C:/temp/image.png"; // Must be a png or jpeg file of less than 8192 kilobytes.
	NewModParams.Summary = "Mod for Upload Sample";

	std::cout << "Using Logo File path : " << NewModParams.PathToLogoFile << std::endl; // Change value of PathToLogoFile if necessary

	// Examples of optional fields you can pass when creating a mod
	NewModParams.Tags = {"SampleMod", "SomeOtherTag"}; // Must match tags configured in game's dashboard
	NewModParams.Visibility = Modio::ObjectVisibility::Public; // Default is public
	// Optional binary metadata
	{
		using namespace std::string_literals;
		NewModParams.MetadataBlob = "Hello\0Embedded\0Null\0World"s; // This could be base64 encoded instead, but a
																	 // std::string literal can contain embedded nulls
	}
	// Optional storage for the mod ID of the newly created mod if SubmitNewModAsync succeeds
	Modio::Optional<Modio::ModID> NewlyCreatedModID;
	Modio::SubmitNewModAsync(
		NewModHandle, NewModParams, [&](Modio::ErrorCode ec, Modio::Optional<Modio::ModID> NewModID) {
			std::cout << "Mod submission response received\n";
			
			if (NewModID.has_value())
			{
				NewlyCreatedModID = NewModID; // Store the new mod ID the server returned for later use
			}
			ModioExample::OnAsyncComplete(ec);
		});

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	}
	
	if (!ModioExample::DidAsyncOperationSucceed())
	{
		return -1;
	}

	// NewlyCreatedModID will only be populated if we successfully created the mod on the server
	if (NewlyCreatedModID.has_value())
	{
		std::cout << "Submitting new modfile for newly created mod" << std::endl;

		Modio::CreateModFileParams Params;

		// Mandatory parameters
		Params.RootDirectory = "C:/temp/mod_folder"; // Must be a folder

		std::cout << "Using Directory for Mod File : " << Params.RootDirectory << std::endl; // Change value of RootDirectory if necessary

		// Optional fields
		Params.bSetAsActive = true; // Sets the new submission as the active modfile, defaults to true
		Params.Platforms = {Modio::ModfilePlatform::Windows,       
							Modio::ModfilePlatform::XboxSeriesX}; // Specify the platforms this modfile targets
		// Params.Platforms must match what is enabled in the platform approval dashboard
		Params.Version = "0.0.1";

		// Use the mod ID returned from SubmitNewModAsync() callback to submit a new file for the new mod
		// This method completes synchronously, and queues the mod for submission at a later time
		// The submission is handled by the SDK's mod management loop, enabled earlier by 'EnableModManagement'
		Modio::SubmitNewModFileForMod(NewlyCreatedModID.value(), Params);

		// Waits until the Mod Management loop processes the file upload and it either succeeds or fails
		while (!UploadCompleted)
		{
			Modio::RunPendingHandlers();
		}
	}
}