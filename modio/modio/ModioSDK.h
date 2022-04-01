/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "ModioGeneratedVariables.h"

#include "modio/detail/ModioDefines.h"

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioCreateModFileParams.h"
#include "modio/core/ModioCreateModParams.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioFilterParams.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioModDependency.h"
#include "modio/core/ModioReportParams.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioModDetails.h"
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/core/entities/ModioModTagOptions.h"
#include "modio/core/entities/ModioTerms.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/ModioLibraryConfigurationHelpers.h"

namespace Modio
{
	/// @docpublic
	/// @brief Initializes the SDK for the given user. Loads the state of mods installed on the system as well as the
	/// set of mods the specified user has installed on this device
	/// @param InitOptions Parameters to the function packed as a struct where all members needs to be initialized for
	/// the call to succeed
	/// @errorcategory NetworkError|Couldn't connect to the mod.io servers
	/// @errorcategory FilesystemError|Couldn't create the user data or common data folders
	/// @errorcategory ConfigurationError|InitOptions contains an invalid value - inspect ec.value() to determine what
	/// was incorrect
	/// @error GenericError::SDKAlreadyInitialized|SDK already initialized
	MODIOSDK_API void InitializeAsync(Modio::InitializeOptions InitOptions,
									  std::function<void(Modio::ErrorCode)> OnInitComplete);

	/// @docpublic
	/// @brief Sets the global logging level - messages with a log level below the specified value will not be displayed
	/// @param Level Value indicating which priority of messages should be included in the log output
	MODIOSDK_API void SetLogLevel(Modio::LogLevel Level);

	/// @docpublic
	/// @brief Provide a callback to handle log messages emitted by the SDK.
	/// @param LogCallback Callback invoked by the SDK during xref:RunPendingHandlers[Modio::RunPendingHandlers] for
	/// each log emitted during that invocation
	MODIOSDK_API void SetLogCallback(std::function<void(Modio::LogLevel, const std::string&)> LogCallback);

	/// @docpublic
	/// @brief Runs any pending SDK work on the calling thread, including invoking any callbacks passed to asynchronous
	/// operations.
	/// NOTE: This should be called while xref:InitializeAsync[Modio::InitializeAsync] and
	/// xref:ShutdownAsync[Modio::ShutdownAsync] are running, as they both utilize the internal event loop for
	/// functionality.
	MODIOSDK_API void RunPendingHandlers();

	/// @docpublic
	/// @brief Cancels any running internal operations, frees SDK resources, and invokes any pending callbacks with
	/// Modio::GenericError::OperationCanceled . This function will block while the deinitialization occurs.
	MODIOSDK_API void Shutdown();

	/// @docpublic
	/// @brief Cancels any running internal operations and invokes any pending callbacks with
	/// Modio::GenericError::OperationCanceled. This function does not block; you should keep calling
	/// Modio::RunPendingHandlers until the callback you provide to this function is invoked.
	/// @param OnShutdownComplete
	/// @requires initialized-sdk
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	MODIOSDK_API void ShutdownAsync(std::function<void(Modio::ErrorCode)> OnShutdownComplete);

	/// @docpublic
	/// @brief Sends a request to the Mod.io server to add the specified mod to the user's list of subscriptions, and
	/// marks the mod for local installation by the SDK
	/// @param ModToSubscribeTo Mod ID of the mod requiring a subscription.
	/// @param OnSubscribeComplete Callback invoked when the subscription request is completed.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @requires management-enabled
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void SubscribeToModAsync(Modio::ModID ModToSubscribeTo,
										  std::function<void(Modio::ErrorCode)> OnSubscribeComplete);

	/// @docpublic
	/// @brief Sends a request to the Mod.io server to remove the specified mod from the user's list of subscriptions.
	/// If no other local users are subscribed to the specified mod this function will also mark the mod for
	/// uninstallation by the SDK.
	/// @param ModToUnsubscribeFrom Mod ID of the mod requiring unsubscription.
	/// @param OnUnsubscribeComplete Callback invoked when the unsubscription request is completed.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @requires management-enabled
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void UnsubscribeFromModAsync(Modio::ModID ModToUnsubscribeFrom,
											  std::function<void(Modio::ErrorCode)> OnUnsubscribeComplete);

	/// @docpublic
	/// @brief Synchronises the local list of the current user's subscribed mods with the server. Any mods that have
	/// been externally subscribed will be automatically marked for installation, and mods that have been externally
	/// removed from the user's subscriptions may be uninstalled if no other local users have a current subscription.
	/// Calling this before you call <<QueryUserSubscriptions>>, <<QueryUserInstallations>> or
	/// <<QuerySystemInstallations>> will ensure that if the system mod directory has been moved or relocated, those
	/// functions will still return correct values.
	/// @param OnFetchDone Callback invoked when the external state has been retrieved and merged with the local data
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void FetchExternalUpdatesAsync(std::function<void(Modio::ErrorCode)> OnFetchDone);

	/// @docpublic
	/// @brief Enables the automatic management of installed mods on the system based on the user's subscriptions.
	/// Does nothing if mod management is currently enabled. Note: this function does not behave like other "async"
	/// methods, given that its name does not include the word async.
	/// @param ModManagementHandler This callback handler will be invoked with a ModManagementEvent for each mod
	/// operation performed by the SDK
	///
	/// @return Modio::ErrorCode indicating if mod management was enabled successfully
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error ModManagementError::ModManagementAlreadyEnabled|Mod management was already enabled. The mod management
	/// callback has not been changed
	MODIOSDK_API Modio::ErrorCode EnableModManagement(
		std::function<void(Modio::ModManagementEvent)> ModManagementHandler);

	/// @docpublic
	/// @brief Disables automatic installation or uninstallation of mods based on the user's subscriptions. Allows
	/// currently processing installation to complete; will cancel any pending operations when called.
	MODIOSDK_API void DisableModManagement();

	/// @docpublic
	/// @brief Checks if the automatic management process is currently installing or removing mods
	/// @return True if automatic management is currently performing an operation
	MODIOSDK_API bool IsModManagementBusy();

	/// @docpublic
	/// @brief Provides progress information for a mod installation or update operation if one is currently in progress.
	/// @return Optional ModProgressInfo object containing information regarding the progress of the installation
	/// operation.
	MODIOSDK_API Modio::Optional<Modio::ModProgressInfo> QueryCurrentModUpdate();

	/// @docpublic
	/// @brief Fetches the local view of the user's subscribed mods, including mods that are subscribed but not yet
	/// installed
	/// @return std::map using Mod IDs as keys and ModCollectionEntry objects providing information about the subscribed
	/// mods
	MODIOSDK_API std::map<Modio::ModID, Modio::ModCollectionEntry> QueryUserSubscriptions();

	/// @docpublic
	/// @brief Fetches the subset of the user's subscribed mods that are installed and therefore ready for loading
	/// @param bIncludeOutdatedMods Include subscribed mods that are installed but have an updated version on the server
	/// that has not yet been installed
	/// @return std::map using Mod IDs as keys and ModCollectionEntry objects providing information about the subscribed
	/// mods
	MODIOSDK_API std::map<Modio::ModID, Modio::ModCollectionEntry> QueryUserInstallations(bool bIncludeOutdatedMods);

	/// @docpublic
	/// @brief Fetches all mods installed on the system such that a consuming application can present the information in
	/// a UI in order to free up space by uninstalling mods
	/// @return std::map using Mod IDs as keys and ModCollectionEntry objects providing information about mods installed
	/// on the system regardless of which user installed them
	MODIOSDK_API std::map<Modio::ModID, Modio::ModCollectionEntry> QuerySystemInstallations();

	/// @docpublic
	/// @brief Forcibly uninstalls a mod from the system. This is intended for use when a host application requires more
	/// room for a mod that the user wants to install, and as such will return an error if the current user is
	/// subscribed to the mod. To remove a mod the current user is subscribed to, use
	/// xref:UnsubscribeFromModAsync[Modio::UnsubscribeFromModAsync].
	/// @param Callback Callback invoked when the uninstallation is successful, or if it failed because the current user
	/// remains subscribed.
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error ModManagementError::AlreadySubscribed|User is still subscribed to the specified mod
	MODIOSDK_API void ForceUninstallModAsync(Modio::ModID ModToRemove, std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Fetches the currently authenticated Mod.io user profile if there is one associated with the current
	/// platform user
	/// @return Optional Modio::User object containing profile information
	MODIOSDK_API Modio::Optional<Modio::User> QueryUserProfile();

	/// @docpublic
	/// @brief Uses platform-specific authentication to associate a Mod.io user account with the current platform user
	/// @param User Authentication payload data to submit to the provider
	/// @param Provider The provider to use to perform the authentication
	/// @param Callback Callback invoked once the authentication request has been made
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires no-authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserAuthError::AlreadyAuthenticated|Authenticated user already signed-in. Call ClearUserDataAsync to
	/// de-authenticate the old user, then Shutdown() and reinitialize the SDK first.
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void AuthenticateUserExternalAsync(Modio::AuthenticationParams User,
													Modio::AuthenticationProvider Provider,
													std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief This function retrieves the information required for a game to display the mod.io terms of use to a
	/// player who wishes to create a mod.io account
	/// @param Provider The provider to use to perform the authentication
	/// @param Locale The language to display the terms of use in
	/// @param Callback Callback invoked with the terms of use data once retrieved from the server
	/// @requires initialized-sdk
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetTermsOfUseAsync(Modio::AuthenticationProvider Provider, Modio::Language Locale,
										 std::function<void(Modio::ErrorCode, Modio::Optional<Modio::Terms>)> Callback);

	/// @docpublic
	/// @brief Returns a handle for use with SubmitNewModAsync
	/// @return The handle to use in a call to SubmitNewModAsync
	MODIOSDK_API Modio::ModCreationHandle GetModCreationHandle();

	/// @docpublic
	/// @brief Requests a new mod be created with the specified properties
	/// @param Handle Handle returned by xref:GetModCreationHandle[Modio::GetModCreationHandle]. Each successful call to
	/// SubmitNewModAsync requires a new ModCreationHandle.
	/// @param Params Information about the mod to be created
	/// @param Callback Callback invoked with the ID of the created mod once the parameters are validated by the server
	/// and the mod is created
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory InvalidArgsError|Some of the information in the ModCreateParams did not pass validation
	/// @error UserDataError::InvalidUser|No authenticated user
	MODIOSDK_API void SubmitNewModAsync(
		Modio::ModCreationHandle Handle, Modio::CreateModParams Params,
		std::function<void(Modio::ErrorCode ec, Modio::Optional<Modio::ModID>)> Callback);

	/// @docpublic
	/// @brief Queues a modfile submission. The submission process creates an archive using the information specified in
	/// the parameters, then uploads that file to the server as a new modfile release for the specified mod. When the
	/// modfile submission completes, a Mod Management Event will be triggered.
	/// @param Mod The mod to attach the modfile to
	/// @param Params Descriptor containing information regarding the modfile
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @requires management-enabled
	MODIOSDK_API void SubmitNewModFileForMod(Modio::ModID Mod, Modio::CreateModFileParams Params);

	/// @docpublic
	/// @brief Provides a list of mods for the current game, that match the parameters specified in the filter
	/// @param Filter Modio::FilterParams object containing any filters that should be applied to the query
	/// @param Callback Callback invoked with a status code and an optional ModInfoList providing mod profiles
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void ListAllModsAsync(
		Modio::FilterParams Filter,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)> Callback);

	/// @docpublic
	/// @brief Fetches detailed information about the specified mod, including description and file metadata for the
	/// most recent release
	/// @param ModId Mod ID of the mod to fetch data
	/// @param Callback Callback providing a status code and an optional Modio::ModInfo object with the mod's extended
	/// information
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetModInfoAsync(Modio::ModID ModId,
									  std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)> Callback);

	/// @docpublic
	/// @brief Downloads the logo for the specified mod. Will use existing file if it is already present on disk
	/// @param ModId Mod ID for use in logo retrieval
	/// @param LogoSize Parameter indicating the size of logo that's required
	/// @param Callback Callback providing a status code and an optional path object pointing to the location of the
	/// downloaded image
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod media does not exist or was deleted
	/// @error FilesystemError::InsufficientSpace|Not enough space for the file
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetModMediaAsync(
		Modio::ModID ModId, Modio::LogoSize LogoSize,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::filesystem::path>)> Callback);

	/// @docpublic
	/// @brief Get a gallery image for the specified mod ID. If it already exists on disk the file will be reused unless
	/// it is outdated
	/// @param ModId The mod you want to retrieve an image for
	/// @param GallerySize Size of the image you want to retrieve
	/// @param Index The zero-based index of the image you want to retrieve
	/// @param Callback Callback containing a status code and an Optional containing a path to the image file on disk
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod media does not exist or was deleted
	/// @error FilesystemError::InsufficientSpace|Not enough space for the file
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetModMediaAsync(
		Modio::ModID ModId, Modio::GallerySize GallerySize, Modio::GalleryIndex Index,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::filesystem::path>)> Callback);

	/// @docpublic
	/// @brief Downloads the creator avatar for a specified mod. Will use existing file if it is already present on disk
	/// and not outdated
	/// @param ModId ID of the mod the creator avatar will be retrieved for
	/// @param AvatarSize Parameter indicating the size of avatar image that's required
	/// @param Callback Callback providing a status code and an optional path object pointing to the location of the
	/// downloaded image
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod media does not exist or was deleted
	/// @error FilesystemError::InsufficientSpace|Not enough space for the file
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetModMediaAsync(
		Modio::ModID ModId, Modio::AvatarSize AvatarSize,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::filesystem::path>)> Callback);

	/// @brief Submits a rating for a mod on behalf of the currently authenticated user.
	/// @param ModID The mod to submit a rating for
	/// @param Rating The rating to submit \r\n NOTE: To clear a rating for a mod, submit a rating of Rating::Neutral.
	/// @param Callback Callback providing a status code to indicate the rating was submitted successfully
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod could not be found
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void SubmitModRatingAsync(Modio::ModID ModID, Modio::Rating Rating,
										   std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Fetches the available tags used on mods for the current game. These tags can them be used in conjunction
	/// with the FilterParams passed to ListAllMods
	/// @param Callback Callback providing a status code and an optional ModTagOptions object containing the available
	/// tags
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetModTagOptionsAsync(
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModTagOptions>)> Callback);

	/// @docpublic
	/// @brief For a given Mod ID, fetches a list of any mods that the creator has marked as dependencies
	/// @param ModID The mod to retrieve dependencies for
	/// @param Callback Callback providing a status code and an optional xref:ModDependencyList[ModDependencyList]
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @experimental
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetModDependenciesAsync(
		Modio::ModID ModID,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDependencyList> Dependencies)> Callback);

	/// @docpublic
	/// @brief Begins email authentication for the current session by requesting a one-time code be sent to the
	/// specified email address if it is associated with a Mod.io account
	/// @param EmailAddress The email address to send the code to
	/// @param Callback Callback providing a status code indicating the outcome of the request
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires no-authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserAuthError::AlreadyAuthenticated|Authenticated user already signed-in. Call ClearUserDataAsync to
	/// de-authenticate the old user, then Shutdown() and reinitialize the SDK first.
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void RequestEmailAuthCodeAsync(Modio::EmailAddress EmailAddress,
												std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Completes email authentication for the current session by submitting the one-time code sent to the user's
	/// email address
	/// @param AuthenticationCode User's authentication code
	/// @param Callback Callback providing a status code indicating if authentication was successful or not
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires no-authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserAuthError::AlreadyAuthenticated|Authenticated user already signed-in. Call ClearUserDataAsync to
	/// de-authenticate the old user, then Shutdown() and reinitialize the SDK first.
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void AuthenticateUserEmailAsync(Modio::EmailAuthCode AuthenticationCode,
												 std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief De-authenticates the current Mod.io user for the current session, and clears all user-specific data
	/// stored on the current device. Any subscribed mods that are installed but do not have other local users
	/// subscribed will be uninstalled
	/// @param Callback Callback providing a status code indicating the outcome of clearing the user data. Error codes
	/// returned by this function are informative only - it will always succeed.
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void ClearUserDataAsync(std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Downloads the avatar of the currently authenticated user. Will only perform a download if there is no
	/// local cache of the avatar or if that cached copy is out-of-date.
	/// @param AvatarSize Parameter specifying the size of avatar image to download
	/// @param Callback Callback providing a status code for the download and an optional path to the downloaded image
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetUserMediaAsync(
		Modio::AvatarSize AvatarSize,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::filesystem::path>)> Callback);

	/// @docpublic
	/// @brief If the last request to the mod.io servers returned a validation failure, this function returns extended
	/// information describing the fields that failed validation.
	/// @return Collection of Modio::FieldError objects, or empty collection if there was no validation failures
	/// @requires initialized-sdk
	MODIOSDK_API std::vector<Modio::FieldError> GetLastValidationError();

	/// @docpublic
	/// @brief Sends a content report to mod.io. When using this function, please inform your users that if they provide
	/// their contact name or details in the Report parameter, that those may be shared with the person responsible for
	/// the content being reported. For more information on what data in a report will be shared with whom, please see
	/// link:https://mod.io/report/widget[our website's report form] for more information.
	/// @param Report Information about the content being reported and a description of the report.
	/// @param Callback Callback providing a status code to indicate successful submission of the report.
	/// @requires initialized-sdk
	/// @errorcategory NetworkError|Couldn't Connect to mod.io servers
	/// @errorcategory InvalidArgsError|Required information in the report did not pass validation
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void ReportContentAsync(Modio::ReportParams Report, std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Submit a new logo for an existing mod, overriding any existing image currently stored on the server. The
	/// logo must be gif, jpg, or png format and cannot exceed 8MB in filesize. Dimensions must be at least 512x288. We
	/// recommended you supply a high resolution image with a 16:9 ratio. mod.io will use this logo to create three
	/// thumbnails with the dimensions of 320x180, 640x360 and 1280x720.
	/// @param ModID ID of the mod whose logo will be added or updated
	/// @param LogoPath Path to the new image file of the logo to be uploaded.
	/// @param Callback Callback providing a status code indicating the outcome of the request
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void AddOrUpdateModLogoAsync(Modio::ModID ModID, Modio::filesystem::path LogoPath,
											  std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Archives a mod. This mod will no longer be able to be viewed or retrieved via the SDK, but it will still
	/// exist should you choose to restore it at a later date. Archiving is restricted to team managers and
	/// administrators only. Note that restoration and permanent deletion of a mod is possible only via web interface.
	/// @param ModID The mod to be archived.
	/// @requires authenticated-user
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @error ApiError::InsufficientPermission|The authenticated user does not have permission to archive this mod. This action
	/// is restricted to team managers and administrators only.
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	MODIOSDK_API void ArchiveModAsync(Modio::ModID ModID, std::function<void(Modio::ErrorCode)> Callback);

} // namespace Modio

// Implementation headers

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/impl/SDKCore.ipp"
	#include "modio/impl/SDKModManagement.ipp"
	#include "modio/impl/SDKModMetadata.ipp"
	#include "modio/impl/SDKUserData.ipp"
#endif

#include "modio/detail/ModioUndefs.h"
