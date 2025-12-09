/*
 *  Copyright (C) 2021-2025 mod.io Pty Ltd. <https://mod.io>
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
#include <future>

#pragma warning(push, 0)

/// This example demonstrates how to refresh a users monetization entitlements. Note that each platform
///	has additional functionality
/// Once these queries have been performed, the sample initiates an async shutdown of the SDK if necessary and then
/// terminates.

/// @brief Current balance of the users wallet
static uint64_t UserWalletBalance;

std::map<Modio::Portal, std::string> ModioPortalToSkuPortalMap = {
	{Modio::Portal::None, "web"},
	{Modio::Portal::Steam, "steam"},
	{Modio::Portal::XboxLive, "xboxlive"},
	{Modio::Portal::PSN, "psn"},
};

static std::vector<Modio::ModMonetizationSKU> GetSkusForPortal(Modio::ModInfo& ModInfo, Modio::Portal Portal)
{
	std::string skuPortal = ModioPortalToSkuPortalMap[Portal];
	if (skuPortal.empty())
	{
		return {};
	}
	std::vector<Modio::ModMonetizationSKU> Skus {};

	for (auto Sku : ModInfo.SKUMappings)
	{
		if (Sku.Portal == skuPortal)
		{
			Skus.push_back(Sku);
		}
	}

	return Skus;
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

struct PlatformSKUMapping
{
	std::string SkuId {};
	std::uint64_t Price =	0;

	void print() const
	{
		std::cout << "SKU Mapping { ID: \"" << SkuId << "\", Price: " << Price << " }\n";
	}
};

static std::promise<void> SteamAuthComplete;
static std::promise<void> SteamRequestPricesComplete;
static std::string EncodedSteamAuth;
static std::string CurrencyCode {};
static std::vector<PlatformSKUMapping> PlatformSKUMappings {};

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
	void OnRequestPricesResponse(SteamInventoryRequestPricesResult_t* result, bool bFailure)
	{
		if (bFailure)
		{
			printf("Failed to fetch Steam SKUs\n");
		}
		else
		{
			printf("Fetch Steam SKUs complete:\n");
		}

		CurrencyCode = result->m_rgchCurrency;

		uint32_t len = SteamInventory()->GetNumItemsWithPrices();

		std::vector<SteamItemDef_t> itemDefs(len);
		std::vector<uint64> prices(len);
		std::vector<uint64> basePrices(len);

		SteamInventory()->GetItemsWithPrices(itemDefs.data(), prices.data(), basePrices.data(), len);

		if (!itemDefs.empty())
		{
			for (uint32_t i = 0; i < itemDefs.size(); i++)
			{
				PlatformSKUMapping skuMapping;
				skuMapping.Price = prices[i];
				skuMapping.SkuId = std::to_string(itemDefs[i]);
				skuMapping.print();
				PlatformSKUMappings.push_back(skuMapping);
			}
		}
		SteamRequestPricesComplete.set_value();
	}

	CCallResult<SteamAuthHelper, SteamInventoryRequestPricesResult_t> m_SteamCallResultInventoryPrices {};
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
	SteamCallbacks->m_SteamCallResultEncryptedAppTicket.Set(hSteamAPICall, SteamCallbacks,
															&SteamAuthHelper::OnEncryptedAppTicketResponse);

	SteamAuthComplete.get_future().wait();

	// Ask user for their input and offer default values
	int GameIDInt = std::stoi(RetrieveUserInput("Game ID:", "3609"));
	std::string APIStr = RetrieveUserInput("API key:", "ca842a1f60c40bc8fb2044bc9932d763");
	std::string ModioEnvStr = RetrieveUserInput("Modio Environment:", "Live");
	std::string SessionID = RetrieveUserInput("SessionID:", "ExampleSession");
	Modio::ModID ModIDInt(std::stoi(RetrieveUserInput("Mod ID:", "1105910")));

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

	// Determine if USD marketplace is enabled for this game.
	{
		std::promise<void> GetGameInfoComplete;
		Modio::GetGameInfoAsync(
			Modio::GameID(GameIDInt), [&](Modio::ErrorCode ec, Modio::Optional<Modio::GameInfo> gameInfo) {
				if (ec)
				{
					std::cout << "Failed to get game info: " << ec.message() << std::endl;
				}
				else
				{
					if (gameInfo->GameMonetizationOptions.HasFlag(Modio::GameMonetizationOptions::MarketplaceUSD))
					{
						std::cout << "USD Marketplace is enabled on this game" << std::endl;
					}
					else
					{
						std::cout << "USD Marketplace is disabled on this game" << std::endl;
					}
				}
				GetGameInfoComplete.set_value();
			});

		GetGameInfoComplete.get_future().wait();
	}

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

	// Cache platform SKU mappings and price info
	{
		SteamAPICall_t hSteamAPICall = SteamInventory()->RequestPrices();
		SteamCallbacks->m_SteamCallResultInventoryPrices.Set(hSteamAPICall, SteamCallbacks,
															 &SteamAuthHelper::OnRequestPricesResponse);

		SteamRequestPricesComplete.get_future().wait();
	}
	
	// Cache mod SKU(s).
	std::vector<Modio::ModMonetizationSKU> ModSkus {};
	{
		std::promise<void> GetModInfoComplete;

		Modio::GetModInfoAsync(ModIDInt, [&](Modio::ErrorCode ec, Modio::Optional<Modio::ModInfo> modinfo) {
			if (ec)
			{
				std::cout << "Failed to get mod info: " << ec.message() << std::endl;
			}
			else
			{
				ModSkus = GetSkusForPortal(*modinfo, Modio::Portal::Steam);
				std::cout << "Get mod SKUs complete" << std::endl;
			}
			GetModInfoComplete.set_value();
		});

		GetModInfoComplete.get_future().wait();
	}

	// Ensure the SKU our Mod requires is available on Steam.
	const PlatformSKUMapping* foundSkuMapping = nullptr;

	for (const Modio::ModMonetizationSKU& modSKU : ModSkus)
	{
		for (const PlatformSKUMapping& steamSKU : PlatformSKUMappings)
		{
			if (modSKU.Sku == steamSKU.SkuId)
			{
				foundSkuMapping = &steamSKU;
			}
		}
	}

	// Simulated purchase flow (click purchase on monetized Mod)
	if (foundSkuMapping)
	{
		std::cout << "SKU match found between Modio & Steam" << std::endl;

		// Check for entitlement
		Modio::EntitlementList CurrentAvailableEntitlements;
		{
			std::promise<void> GetAvailableUserEntitlementsComplete;

			Modio::GetAvailableUserEntitlementsAsync(
				Modio::EntitlementParams {},
				[&](Modio::ErrorCode ec, Modio::Optional<Modio::EntitlementList> AvailableEntitlements) {
					if (!ec)
					{
						CurrentAvailableEntitlements = *AvailableEntitlements;
						std::cout << "Get available user entitlements complete" << std::endl;
					}
					else
					{
						std::cout << "Failed to get user entitlements: " << ec.message() << std::endl;
					}
					GetAvailableUserEntitlementsComplete.set_value();
				});

			GetAvailableUserEntitlementsComplete.get_future().wait();
		}

		// If missing, prompt user to purchase SKu from platform
		bool entitlementFound = false;
		for (const Modio::Entitlement& entitlement : CurrentAvailableEntitlements)
		{
			if (entitlement.SkuId == foundSkuMapping->SkuId)
			{
				entitlementFound = true;
				std::cout << "User already has required entitlement." << std::endl;
			}
		}
		if (!entitlementFound)
		{
			std::cout << "User is missing required entitlement, prompting purchase..." << std::endl;
			std::stringstream skuURL;
			skuURL << "https://store.steampowered.com/buyitem/1550360/" << foundSkuMapping->SkuId << "/1";

			SteamFriends()->ActivateGameOverlayToWebPage(skuURL.str().c_str());

			// @note You would likely want to implement a notification for if the purchase has been successful.
		}

		// Attempt purchase mod with entitlement
		// Modio::PurchaseModWithEntitlementAsync(ModIDInt, Modio::EntitlementParams(), nullptr);
	}
	else
	{
		std::cout << "No matching SKU found between Steam & desired mod" << std::endl;
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

#pragma warning(pop)