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

#include "isteamfriends.h"
#include "steam_api.h"

#include <iostream>
#include <sstream>
#include <utility>

/// This example demonstrates how to refresh a users monetization entitlements. Note that each platform
///	has additional functionality
/// Once these queries have been performed, the sample initiates an async shutdown of the SDK if necessary and then
/// terminates.

/// @brief Current balance of the users wallet
static uint64_t UserWalletBalance;

/// @brief Helper method to retrieve input from a user with an string prompt
/// @param Prompt The string to show related to the expected user input
/// @param DefaultValue If the user wants to use a default input, this will be used
/// @return String with the user input or default value
static std::string RetrieveUserInput(std::string Prompt, std::string DefaultValue = "")
{
	std::string UserInput = "";
	std::cout << Prompt << std::endl;

	if (!DefaultValue.empty())
	{
		std::cout << "(To use the default value \"" << DefaultValue << "\", just tap Enter)" << std::endl;
	}
	std::getline(std::cin, UserInput);

	if (!UserInput.empty())
	{
		return DefaultValue;
	}
	else
	{
		return UserInput;
	}
}

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
										"abcdefghijklmnopqrstuvwxyz"
										"0123456789+/";

std::string base64_encode(unsigned char* bytes_to_encode, unsigned int in_len)
{
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';
	}

	return ret;
}

static std::promise<void> SteamAuthComplete;
static std::string EncodedSteamAuth;

class SteamAuthHelper
{
public:
	void OnEncryptedAppTicketResponse(EncryptedAppTicketResponse_t* pEncryptedAppTicketResponse, bool bIOFailure)
	{
		switch (pEncryptedAppTicketResponse->m_eResult)
		{
			case k_EResultOK:
			{
				unsigned char rgubTicket[1024];
				uint32 cubTicket;
				if (SteamUser()->GetEncryptedAppTicket(rgubTicket, sizeof(rgubTicket), &cubTicket))
				{
					EncodedSteamAuth = base64_encode(rgubTicket, cubTicket);
					std::cout << "Steam App Ticket received" << std::endl;
				}
				else
				{
					printf("GetEncryptedAppTicket failed.\n");
				}
			}
			break;
			case k_EResultNoConnection:
				printf("Calling RequestEncryptedAppTicket while not connected to steam results in this error.\n");
				break;
			case k_EResultDuplicateRequest:
				printf("Calling RequestEncryptedAppTicket while there is already a pending request results in this "
					   "error.\n");
				break;
			case k_EResultLimitExceeded:
				printf("Calling RequestEncryptedAppTicket more than once per minute returns this error.\n");
				break;
		}

		SteamAuthComplete.set_value();
	}

	CCallResult<SteamAuthHelper, EncryptedAppTicketResponse_t> m_SteamCallResultEncryptedAppTicket;
};

int main()
{

	std::atomic<bool> bHaltBackgroundThread {false};

	SteamAuthHelper* SteamCallbacks = new SteamAuthHelper();

	// Initialize the Steam API
	if (!SteamAPI_Init())
	{
		printf("SteamAPI_Init() failed\n");
		return -1;
	}
	else
	{
		printf("SteamAPI_Init() returned true\n");
	}

	// Set up a background thread to run
	std::thread HandlerThread = std::thread([&]() {
		while (!bHaltBackgroundThread)
		{
			Modio::RunPendingHandlers();
			SteamAPI_RunCallbacks();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		bHaltBackgroundThread = false;
	});

	// Get the Steam Encrypted App Ticket
	uint32 k_unSecretData = 0x5444;
	SteamAPICall_t hSteamAPICall = SteamUser()->RequestEncryptedAppTicket(&k_unSecretData, sizeof(k_unSecretData));
	SteamCallbacks->m_SteamCallResultEncryptedAppTicket.Set(hSteamAPICall, SteamCallbacks, &SteamAuthHelper::OnEncryptedAppTicketResponse);

	SteamAuthComplete.get_future().wait();

	// Ask user for their input and offer default values
	std::stringstream IntTransformer;
	IntTransformer << RetrieveUserInput("Game ID:", "3609");
	std::string APIStr = RetrieveUserInput("API key:", "ca842a1f60c40bc8fb2044bc9932d763");
	std::string ModioEnvStr = RetrieveUserInput("Modio Environment:", "Live");
	std::string SessionID = RetrieveUserInput("SessionID:", "ExampleSession");

	// Transform GameID to integer
	int GameIDInt = 0;
	IntTransformer >> GameIDInt;

	// Determine the modio API environment to use
	Modio::Environment ModioEnv = Modio::Environment::Live;

	std::map<std::string, std::string> ExtendedParams;

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

	Modio::InitializeOptions InitOptions;
	InitOptions.GameID = Modio::GameID(GameIDInt);
	InitOptions.APIKey = Modio::ApiKey(APIStr);
	InitOptions.GameEnvironment = ModioEnv;
	InitOptions.User = SessionID;
	InitOptions.ExtendedParameters = ExtendedParams;

	// Ensure that the Portal is set to Steam for entitlement consumption
	InitOptions.PortalInUse = Modio::Portal::Steam;

	Modio::InitializeAsync(InitOptions, [&](Modio::ErrorCode ec) {
		if (ec)
		{
			std::cout << "Failed to initialize mod.io SDK: " << ec.message() << std::endl;
		}
		else
		{
			std::cout << "Initialization complete" << std::endl;
		}
		InitializeComplete.set_value();
	});

	InitializeComplete.get_future().wait();

	// Perform Steam SSO
	{
		std::promise<void> AuthenticationComplete;

		Modio::AuthenticationParams AuthParams;
		AuthParams.AuthToken = EncodedSteamAuth;
		AuthParams.bURLEncodeAuthToken = true;
		AuthParams.bUserHasAcceptedTerms = true;
		Modio::AuthenticateUserExternalAsync(
			AuthParams, Modio::AuthenticationProvider::Steam, [&](Modio::ErrorCode ec) {
				if (ec)
				{
					std::cout << "Failed to auth mod.io to mod.io: " << ec.message() << std::endl;
				}
				else
				{
					std::cout << "Authentication complete" << std::endl;
				}
				AuthenticationComplete.set_value();
			});

		AuthenticationComplete.get_future().wait();
	}

	// Get the user's wallet balance for the current game. Note that if a wallet does not exist for a user, this call
	// will automatically create the wallet for them.
	{
		std::promise<void> GetWalletBalanceComplete;
		Modio::GetUserWalletBalanceAsync([&](Modio::ErrorCode ec, Modio::Optional<uint64_t> WalletBalance) {
			if (ec)
			{
				std::cout << "Failed to get user's wallet balance: " << ec.message() << std::endl;
			}
			else
			{
				if (WalletBalance.has_value())
				{
					UserWalletBalance = WalletBalance.value();
				}

				std::cout << "Users wallet balance is " << UserWalletBalance << std::endl;
			}

			GetWalletBalanceComplete.set_value();
		});

		GetWalletBalanceComplete.get_future().wait();
	}

	// After you have called GetWalletBalanceAsync at startup, you should call RefreshUserEntitlementsAsync
	// in case there are any unconsumed entitlements that the user has purchased - for instance, making a platform
	// purchase via a platform companion app or website outside of the game. Note that you only need to call
	// RefreshUserEntitlementsAsync if you are selling virtual currency packs via a platform store, such as Steam,
	// Playstation Store or XBox Store. If you are only allowing your users to purchase VC Packs via the mod.io website,
	// they are directly added to the users wallet and you do not need to call this.
	{
		std::promise<void> RefreshUserEntitlementsComplete;

		const Modio::EntitlementParams EntitlementParams;
		Modio::RefreshUserEntitlementsAsync(
			EntitlementParams,
			[&](Modio::ErrorCode ec, Modio::Optional<Modio::EntitlementConsumptionStatusList> Entitlements) {
				if (ec)
				{
					std::cout << "Failed to refresh user entitlements: " << ec.message() << std::endl;
				}
				else
				{
					if (Entitlements.has_value() && Entitlements->Size() > 0)
					{
						if (Entitlements->WalletBalance.has_value())
						{
							UserWalletBalance = Entitlements->WalletBalance->Balance;

							std::cout << "Entitlements consumed: " << Entitlements->Size() << std::endl;
							std::cout << "Updated UserWalletBalance is " << UserWalletBalance;
						}
					}
					else
					{
						std::cout << "No entitlements synced; nothing further to do." << std::endl;
					}
				}

				RefreshUserEntitlementsComplete.set_value();
			});

		RefreshUserEntitlementsComplete.get_future().wait();
	}

	// Request the SDK begins an async shutdown by invoking Modio::ShutdownAsync
	// Parameters:
	// Callback Callback invoked when ShutdownAsync has completed. It takes a single ErrorCode parameter
	// indicating whether an error occurred.
	// Always finalize the SDK.
	std::promise<void> ShutdownComplete;
	Modio::ShutdownAsync([&](Modio::ErrorCode ec) { ShutdownComplete.set_value(); });
	ShutdownComplete.get_future().wait();

	// Halt execution of Modio::RunPendingHandlers(), and wait for the background thread to finish
	bHaltBackgroundThread.store(true);
	if (HandlerThread.joinable())
	{
		HandlerThread.join();
	}
}
