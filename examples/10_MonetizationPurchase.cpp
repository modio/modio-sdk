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

/// This example demonstrates purchasing a mod
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

/// @brief Current balance of the users wallet
static uint64_t UserWalletBalance;

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

	static void OnGetWalletBalanceCompleted(Modio::ErrorCode ec, Modio::Optional<uint64_t> WalletBalance)
	{
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			UserWalletBalance = WalletBalance.value();
			std::cout << "Users wallet balance is: " << std::to_string(UserWalletBalance) << std::endl;
			NotifyApplicationSuccess();
		}
	}

	static void OnRefreshUserEntitlementsCompleted(Modio::ErrorCode ec, Modio::Optional<Modio::EntitlementConsumptionStatusList> EntitlementConsumptionStatus)
	{
		if (ec)
		{
			NotifyApplicationFailure(ec);
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

			// 

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

	static void OnPurchaseComplete(Modio::ErrorCode ec, Modio::Optional<Modio::TransactionRecord> TransactionRecord)
	{
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			// The Transaction Record will contain an updated Wallet Balance, where you can update your local wallet balance state.
			UserWalletBalance = TransactionRecord.value().UpdatedUserWalletBalance;

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
	}

	// Modio::InitializeAsync completed, but we now need to check if it resulted in a success or a failure
	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// SDK initialization failed - this simple sample returns immediately, no need to call ShutdownAsync
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
			// AuthenticateUserEmailAsync. If this function returns successfully, the mod.io account will be stored in
			// the Local User Session passed into InitializeAsync earlier.
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
	}

	// Get the user's wallet balance for the current game. Note that if a wallet does not exist for a user, this call will automatically
	// create the wallet for them.
	{
		Modio::GetUserWalletBalanceAsync(ModioExample::OnGetWalletBalanceCompleted);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		}

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// Error handling for GetWalletBalance errors
		}
	}

	// After you have called GetWalletBalanceAsync at startup, you should call RefreshUserEntitlementsAsync
	// in case there are any unconsumed entitlements that the user has purchased - for instance, making a platform purchase
	// via a platform companion app or website outside of the game.
	// Note that you only need to call RefreshUserEntitlementsAsync if you are selling virtual currency packs via a platform store,
	// such as Steam, Playstation Store or XBox Store. If you are only allowing your users to purchase VC Packs via the mod.io website,
	// they are directly added to the users wallet and you do not need to call this.
	{
		const Modio::EntitlementParams EntitlementParams;
		Modio::RefreshUserEntitlementsAsync(EntitlementParams, ModioExample::OnRefreshUserEntitlementsCompleted);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		}

		if (!ModioExample::DidAsyncOperationSucceed())
		{
			// Error handling for RefreshUserEntitlementsAsync errors
		}
	}

	// Search for all Paid mods so a user can purchase one
	{
		// Set a Search filter to return only Paid mods. You can also use RevenueFilterType::FreeAndPaid to get both free and paid mods
		Modio::FilterParams Filter;
		Filter = Filter.RevenueType(Modio::FilterParams::RevenueFilterType::Paid);
		Modio::ListAllModsAsync(Filter, ModioExample::OnListAllModsComplete);

		while (!ModioExample::HasAsyncOperationCompleted())
		{
			Modio::RunPendingHandlers();
		};

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
					// You can use CurrentModInfo.Price to display the price of the mod.
					std::cout << CurrentModInfo.ModId << "\t" << CurrentModInfo.ProfileName << "\t"
							  << CurrentModInfo.Price << std::endl;
				}
			}
		}
	}

	// Prompt the user for a Mod ID to purchase
	{
		std::int64_t UserModID = -1;
		std::string UserModString = ModioExample::RetrieveUserInput("Enter the ID of the mod you wish to purchase:");

		try
		{
			UserModID = std::stoll(UserModString);
		}
		catch (const std::exception&)
		{
			std::cout << "Invalid Input" << std::endl;
			return -1;
		}

		// In order to purchase a mod, you need the full ModInfo, as you must provide the expected price of the mod,
		// along with the Mod ID.
		Modio::ModInfo ModToPurchase;
		for (auto &ModInfo : Example.LastResults.value())
		{
			if (ModInfo.ModId == UserModID)
			{
				ModToPurchase = ModInfo;
				break;
			}
		}

		if (UserModID != -1)
		{
			std::cout << "Installing Mod ID:" << UserModID << std::endl;

			// Purchase the specific mod.
			// Note that you need to provide the price the user is expecting to pay for the mod. If the expected price
			// and the actual price on the server do not match, you will receive an error on purchase.
			Modio::PurchaseModAsync(Modio::ModID(UserModID), ModToPurchase.Price, ModioExample::OnPurchaseComplete);

			while (!ModioExample::HasAsyncOperationCompleted())
			{
				Modio::RunPendingHandlers();
			};

			if (!ModioExample::DidAsyncOperationSucceed())
			{
				// No need for explicit handling of an error from SubscribeToModAsync in this simple sample
			}
			else
			{
				// Wait for the mod management system to finish installing the new mod and anything else which is
				// pending
				while (Modio::IsModManagementBusy())
				{
					Modio::RunPendingHandlers();
				}
			}
		}
	}

	// Make sure Mod Management is enabled in order to purchase
	if (Modio::ErrorCode ec = Modio::EnableModManagement(ModioExample::OnModManagementEvent))
	{
		// An error occurred, bail early
		std::cout << "Could not enable mod management: " << ec.message() << std::endl;
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

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// No need for explicit handling of an error from ShutdownAsync, the application is about to exit
	}
}
