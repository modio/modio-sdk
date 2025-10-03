/*
 *  Copyright (C) 2021-2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioSplitCompilation.h"
#include "modio/detail/ModioDefines.h"

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioCreateModFileParams.h"
#include "modio/core/ModioCreateModParams.h"
#include "modio/core/ModioCreateSourceFileParams.h"
#include "modio/core/ModioEditModParams.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioFilterParams.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioModDependency.h"
#include "modio/core/ModioReportParams.h"
#include "modio/core/ModioServerInitializeOptions.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioEntitlement.h"
#include "modio/core/entities/ModioEntitlementConsumptionStatusList.h"
#include "modio/core/entities/ModioGameInfo.h"
#include "modio/core/entities/ModioGameInfoList.h"
#include "modio/core/entities/ModioModCollection.h"
#include "modio/core/entities/ModioModCommunityOptions.h"
#include "modio/core/entities/ModioModDetails.h"
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/core/entities/ModioModTagOptions.h"
#include "modio/core/entities/ModioTerms.h"
#include "modio/core/entities/ModioTransactionRecord.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/core/entities/ModioUserList.h"
#include "modio/core/entities/ModioUserRatingList.h"
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
	/// @param LogCallback Callback invoked by the SDK during [`Modio::RunPendingHandlers`](#runpendinghandlers) for
	/// each log emitted during that invocation
	MODIOSDK_API void SetLogCallback(std::function<void(Modio::LogLevel, const std::string&)> LogCallback);

	/// @docpublic
	/// @brief Runs any pending SDK work on the calling thread, including invoking any callbacks passed to asynchronous
	/// operations.
	/// NOTE: This should be called while [`Modio::InitializeAsync`](#initializeasync) and
	/// [`Modio::ShutdownAsync`](#shutdownasync) are running, as they both utilize the internal event loop for
	/// functionality.
	/// NOTE: `RunPendingHandlers` should never be called inside a callback you provide to the SDK. This will result in
	/// a deadlock.
	MODIOSDK_API void RunPendingHandlers();

	/// @docpublic
	/// @brief Cancels any running internal operations and invokes any pending callbacks with
	/// Modio::GenericError::OperationCanceled. This function does not block; you should keep calling
	/// Modio::RunPendingHandlers until the callback you provide to this function is invoked.
	/// NOTE: `ShutdownAsync` should *never* be called inside a callback you provide to the SDK. This will result in a
	/// deadlock.
	/// @param OnShutdownComplete
	/// @requires initialized-sdk
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	MODIOSDK_API void ShutdownAsync(std::function<void(Modio::ErrorCode)> OnShutdownComplete);

	/// @docpublic
	/// @brief Sends a request to the mod.io server to add the specified mod to the user's list of subscriptions, and
	/// marks the mod for local installation by the SDK
	/// @param ModToSubscribeTo Mod ID of the mod requiring a subscription.
	/// @param IncludeDependencies If this mod has any dependencies, all of those will also be subscribed to.
	/// @param OnSubscribeComplete Callback invoked when the subscription request is completed.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @requires management-enabled
	/// @requires mod-not-pending-uninstall
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error ModManagementError::ModBeingProcessed|Specified mod is pending uninstall. Wait until the uninstall
	/// process is complete before subscribing again.
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void SubscribeToModAsync(Modio::ModID ModToSubscribeTo, bool IncludeDependencies,
										  std::function<void(Modio::ErrorCode)> OnSubscribeComplete);

	/// @docpublic
	/// @brief Sends a request to the mod.io server to remove the specified mod from the user's list of subscriptions.
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
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
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
	/// @requires management-enabled
	/// @requires fetch-external-updates-not-running
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::RequestInProgress|FetchExternalUpdates operation is already running.
	MODIOSDK_API void FetchExternalUpdatesAsync(std::function<void(Modio::ErrorCode)> OnFetchDone);

	/// @docpublic
	/// @brief Retrieve a list of updates between the users local mod state, and the server-side state. This allows you
	/// to identify which mods will be modified by the next call to <<FetchExternalUpdatesAsync>> in order to perform
	/// any content management (such as unloading files) that might be required.
	/// @param OnPreviewDone Callback invoked when the external state has been retrieved. It contains a dictionary with
	/// ModID as keys and changes as values. Empty when there are no differences between local and the mod.io API
	/// service
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void PreviewExternalUpdatesAsync(
		std::function<void(Modio::ErrorCode, std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType>)>
			OnPreviewDone);

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
	/// @brief Prioritizes the specified mod for upload, install, or update.  The priority mod will be processed
	/// immediately after the current upload or install completes (if applicable). Only one mod ID can be prioritized at
	/// a time.
	/// @param IDToPrioritize The ID for the mod to prioritize. This mod must be pending upload, install, or update.
	/// @return Error code indicating there is no pending upload, install, or update associated with the provided
	/// priority mod ID.
	/// @error GenericError::BadParameter|The supplied mod ID is invalid or not present in the list of pending
	/// operations
	MODIOSDK_API Modio::ErrorCode PrioritizeTransferForMod(Modio::ModID IDToPrioritize);

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
	/// @remark <<QueryUserInstallations>> is more relevant for most cases to personalize the content shown to the user.
	/// On the other hand, a call to <<QuerySystemInstallations>> returns all mods installed on the system (including
	/// those the current user is subscribed to). This provides insight to mods installed by other users. In case local
	/// space is a concern, here are some options to manage storage:
	/// - Execute <<QuerySystemInstallations>>, let the user know space is limited and provide the chance to select mods
	/// to uninstall. Then call <<ForceUninstallModAsync>> to remove mods selected by the user.
	/// - Execute <<QueryUserInstallations>> and prompt the user to unsubscribe from large mods.
	MODIOSDK_API std::map<Modio::ModID, Modio::ModCollectionEntry> QuerySystemInstallations();

	/// @docpublic
	/// @brief Retrieves a snapshot of current storage related information such as space consumed by mod
	/// installations and total available space
	/// @return Structure containing storage information
	MODIOSDK_API Modio::StorageInfo QueryStorageInfo();

	/// @docpublic
	/// @brief Forcibly uninstalls a mod from the system. This is intended for use when a host application requires more
	/// room for a mod that the user wants to install, and as such will return an error if the current user is
	/// subscribed to the mod. To remove a mod the current user is subscribed to, use
	/// [`Modio::UnsubscribeFromModAsync`](#unsubscribefrommodasync).
	/// @note This function reports its outcome (success or failure) exclusively through the provided `Callback`.
	/// It **does not** emit a separate `Uninstalled` event.
	/// @param ModToRemove The ID for the mod to force remove.
	/// @param Callback Callback invoked when the uninstallation is successful, or if it failed because the current user
	/// remains subscribed.
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error ModManagementError::AlreadySubscribed|User is still subscribed to the specified mod
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void ForceUninstallModAsync(Modio::ModID ModToRemove, std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Queries the server to verify the state of the currently authenticated user if there is one present. An
	/// empty ErrorCode passed to the callback indicates successful verification, i.e. the mod.io server was contactable
	/// and the user's authentication remains valid.
	/// @param Callback Callback invoked once the server-side state has been queried
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	MODIOSDK_API void VerifyUserAuthenticationAsync(std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Queries the server to update the user data.
	/// An empty ErrorCode passed to the callback indicates successful verification, i.e. the mod.io server was
	/// contactable and the user's data got updated.
	/// @param Callback Callback invoked once the server-side state has been queried
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void RefreshUserDataAsync(std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Fetches the currently authenticated mod.io user profile if there is one associated with the current
	/// platform user
	/// @return Optional Modio::User object containing profile information
	MODIOSDK_API Modio::Optional<Modio::User> QueryUserProfile();

	/// @docpublic
	/// @brief Uses platform-specific authentication to associate a mod.io user account with the current platform user
	/// @param User Authentication payload data to submit to the provider. Any AuthToken string that contains special
	/// characters (ex. "+, /, =") requires the boolean "bURLEncodeAuthToken" set as "True" to encode the text
	/// accordingly
	/// @param Provider The provider to use to perform the authentication
	/// @param Callback Callback invoked once the authentication request has been made
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires no-authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @errorcategory ConfigurationError|The SDK's configuration is not valid
	/// @errorcategory InvalidArgsError|The arguments passed to the function have failed validation
	/// @errorcategory UserTermsOfUseError|The user has not yet accepted the mod.io Terms of Use
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void AuthenticateUserExternalAsync(Modio::AuthenticationParams User,
													Modio::AuthenticationProvider Provider,
													std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief This function retrieves the information required for a game to display the mod.io terms of use to a
	/// player who wishes to create a mod.io account
	/// @param Callback Callback invoked with the terms of use data once retrieved from the server
	/// @requires initialized-sdk
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetTermsOfUseAsync(std::function<void(Modio::ErrorCode, Modio::Optional<Modio::Terms>)> Callback);

	/// @docpublic
	/// @brief Returns a handle for use with SubmitNewModAsync
	/// @return The handle to use in a call to SubmitNewModAsync
	MODIOSDK_API Modio::ModCreationHandle GetModCreationHandle();

	/// @docpublic
	/// @brief Requests a new mod be created with the specified properties
	/// @param Handle Handle returned by [`Modio::GetModCreationHandle`](#getmodcreationhandle). Each successful call to
	/// SubmitNewModAsync requires a new ModCreationHandle.
	/// @param Params Information about the mod to be created
	/// @param Callback Callback invoked with the ID of the created mod once the parameters are validated by the server
	/// and the mod is created
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory InvalidArgsError|Some of the information in the CreateModParams did not pass validation
	/// @error UserDataError::InvalidUser|No authenticated user
	MODIOSDK_API void SubmitNewModAsync(
		Modio::ModCreationHandle Handle, Modio::CreateModParams Params,
		std::function<void(Modio::ErrorCode ec, Modio::Optional<Modio::ModID>)> Callback);

	/// @docpublic
	/// @brief Edits the parameters of a mod, by updating any fields set in the Params object to match the passed-in
	/// values. Fields left empty on the Params object will not be updated. If no fields are populated, will return an
	/// error.
	/// @param Mod The mod to edit
	/// @param Params Descriptor containing optional fields indicating what properties should be updated
	/// @param Callback Callback invoked with the updated mod profile on success
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory InvalidArgsError|Some of the information in the EditModParams did not pass validation
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error GenericError::BadParameter|No fields selected for modification in Params, or the supplied mod ID is
	/// invalid
	MODIOSDK_API void SubmitModChangesAsync(
		Modio::ModID Mod, Modio::EditModParams Params,
		std::function<void(Modio::ErrorCode ec, Modio::Optional<Modio::ModInfo>)> Callback);

	/// @docpublic
	/// @brief Queues the upload of a new modfile release for the specified mod using the submitted parameters. This
	/// function takes a Modio::CreateModFileParams object specifying a path to the root folder of the new modfile. The
	/// SDK will compress the folder's contents into a .zip archive and queue the result for upload. When the upload
	/// completes, a Mod Management Event will be triggered. Note the SDK is also responsible for decompressing the
	/// archive upon its installation at a later point in time.
	/// @param Mod The mod to attach the modfile to
	/// @param Params Descriptor containing information regarding the modfile
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @requires management-enabled
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void SubmitNewModFileForMod(Modio::ModID Mod, Modio::CreateModFileParams Params);

	/// @docpublic
	/// @brief Queues the upload of source files for a modfile to defined platforms. This function
	/// takes a Modio::CreateSourceFileParams struct that defines the options needed to upload the source
	/// files. The files present in the root folder defined in the params will be compressed to a .zip archive and
	/// uploaded.
	/// @param Mod The mod to upload source files for
	/// @param Params Descriptor containing information regarding the modfile
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @requires management-enabled
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void SubmitNewModSourceFile(Modio::ModID Mod, Modio::CreateSourceFileParams Params);

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
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
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
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void GetModMediaAsync(Modio::ModID ModId, Modio::LogoSize LogoSize,
									   std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback);

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
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void GetModMediaAsync(Modio::ModID ModId, Modio::GallerySize GallerySize, Modio::GalleryIndex Index,
									   std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback);

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
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void GetModMediaAsync(Modio::ModID ModId, Modio::AvatarSize AvatarSize,
									   std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback);

	/// @docpublic
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
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
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
	/// @param Recursive Include child dependencies in a recursive manner. \r\n NOTE: Recursion supports a maximum depth
	/// of 5.
	/// @param Callback Callback providing a status code and an optional
	/// [`Modio::ModDependencyList`](#ModDependencyList)
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void GetModDependenciesAsync(
		Modio::ModID ModID, bool Recursive,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDependencyList> Dependencies)> Callback);

	/// @docpublic
	/// @brief Adds dependencies to a specified mod, linking it with other mods that are required for it to function
	/// @param ModID The mod to add dependencies to
	/// @param Dependencies List of mod dependencies to add
	/// @param Callback Callback providing a status code to indicate if the dependencies were added successfully
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod could not be found
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void AddModDependenciesAsync(Modio::ModID ModID, std::vector<Modio::ModID> Dependencies,
											  std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Deletes dependencies from a specified mod, unlinking it from other mods that are no longer required.
	/// @param ModID The mod to delete dependencies from
	/// @param Dependencies List of mod IDs to delete as dependencies
	/// @param Callback Callback providing a status code to indicate if the dependencies were deleted successfully
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod IDs are invalid
	MODIOSDK_API void DeleteModDependenciesAsync(Modio::ModID ModID, std::vector<Modio::ModID> Dependencies,
												 std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Begins email authentication for the current session by requesting a one-time code be sent to the
	/// specified email address if it is associated with a mod.io account
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
	/// @brief De-authenticates the current mod.io user for the current session, and clears all user-specific data
	/// stored on the current device. Any subscribed mods that are installed but do not have other local users
	/// subscribed will be marked for uninstallation. This method also disables mod management.
	/// @param Callback Callback providing a status code indicating the outcome of clearing the user data. Error codes
	/// returned by this function are informative only - it will always succeed.
	/// @requires initialized-sdk
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
	MODIOSDK_API void GetUserMediaAsync(Modio::AvatarSize AvatarSize,
										std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback);

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
	/// link: [our website's report form](https://mod.io/report) for more information.
	/// @param Report Information about the content being reported and a description of the report.
	/// @param Callback Callback providing a status code to indicate successful submission of the report.
	/// @requires initialized-sdk
	/// @errorcategory NetworkError|Couldn't Connect to mod.io servers
	/// @errorcategory InvalidArgsError|Required information in the report did not pass validation
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The mod ID, game ID, or user ID supplied to ReportParams is invalid
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
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void AddOrUpdateModLogoAsync(Modio::ModID ModID, std::string LogoPath,
											  std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Add or update images in the gallery for an existing mod. Images must be gif, jpg, or png format and
	/// cannot exceed 8MB in filesize.
	/// @param ModID ID of the mod whose gallery images will be added or updated
	/// @param ImagePaths Vector of paths to the image files to be uploaded
	/// @param SyncGallery If true, existing gallery images will be replaced; if false, new images will be appended
	/// @param Callback Callback providing a status code indicating the outcome of the request
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void AddOrUpdateModGalleryImagesAsync(Modio::ModID ModID, std::vector<std::string> ImagePaths,
													   bool SyncGallery,
													   std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Archives a mod. This mod will no longer be able to be viewed or retrieved via the SDK, but it will still
	/// exist should you choose to restore it at a later date. Archiving is restricted to team managers and
	/// administrators only. Note that restoration and permanent deletion of a mod is possible only via web interface.
	/// @param ModID The mod to be archived.
	/// @requires authenticated-user
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @error ApiError::InsufficientPermission|The authenticated user does not have permission to archive this mod.
	/// This action is restricted to team managers and administrators only.
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	/// @error APIError::CannotArchiveModWithDependents|This mod is a dependency of other mods and as such cannot be
	/// archived
	MODIOSDK_API void ArchiveModAsync(Modio::ModID ModID, std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Provides a list of mods that the user has submitted, or is a team member for, for the current game,
	/// applying the parameters specified in the filter
	/// @param Filter Modio::FilterParams object containing any filters that should be applied to the query
	/// @param Callback Callback invoked with a status code and an optional ModInfoList providing mod profiles
	/// @requires authenticated-user
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void ListUserCreatedModsAsync(
		Modio::FilterParams Filter,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)> Callback);

	/// @docpublic
	/// @brief Returns a list of base mod installation directories. Under normal circumstances, this will return a
	/// single directory, which is the base directory that all mods are installed to for the current user.
	/// @requires initialized-sdk
	/// @return List of base mod installation directories
	MODIOSDK_API std::vector<std::string> GetBaseModInstallationDirectories();

	/// @docpublic
	/// @brief Returns the default mod installation directory for this game and platform, ignoring overrides and without
	/// requiring the SDK to be initialized.
	/// @param GameID The Modio::GameID of the game we're fetching the default mod installation directory for.
	/// @return The default mod installation directory for the specified game on the current platform
	MODIOSDK_API std::string GetDefaultModInstallationDirectory(Modio::GameID GameID);

	/// @docpublic
	/// @brief Mute a user. This will prevent mod.io from returning mods authored by the muted user.
	///	when performing searches.
	/// @requires authenticated-user
	/// @requires initialized-sdk
	/// @param UserID ID of the User to mute
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error GenericError::BadParameter|The supplied user ID is invalid
	MODIOSDK_API void MuteUserAsync(Modio::UserID UserID, std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Unmute a user. This will allow mod.io to mods authored by the previously muted user.
	///	when performing searches.
	/// @requires authenticated-user
	/// @requires initialized-sdk
	/// @param UserID ID of the User to unmute
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error GenericError::BadParameter|The supplied user ID is invalid
	MODIOSDK_API void UnmuteUserAsync(Modio::UserID UserID, std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief List all the users that have been muted by the current user.
	/// @requires authenticated-user
	/// @requires initialized-sdk
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	MODIOSDK_API void GetMutedUsersAsync(
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)> Callback);

	/// @docpublic
	/// @brief Fetches detailed information about the specified game
	/// @param GameID Game ID of the game data to fetch
	/// @param Callback Callback providing a status code and an optional Modio::GameInfo object to store the games's
	/// extended information
	/// @requires initialized-sdk
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @errorcategory EntityNotFoundError|Specified game does not exist
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error GenericError::BadParameter|The supplied game ID is invalid
	MODIOSDK_API void GetGameInfoAsync(
		Modio::GameID GameID, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfo>)> Callback);

	/// @docpublic
	/// @brief Provides a list of games for the current user, that match the parameters specified in the filter
	/// @param Filter Modio::FilterParams object containing any filters that should be applied to the query
	/// @param Callback Callback invoked with a status code and an optional GameInfoList providing game infos
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires user-authenticated
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void ListUserGamesAsync(
		Modio::FilterParams Filter,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfoList>)> Callback);

	/// @docpublic
	/// @brief Provides a list of mod ratings for the current user
	/// @param Callback Callback invoked with a status code and an optional UserRatingList providing ratings for each
	/// mod
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires user-authenticated
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void GetUserRatingsAsync(
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserRatingList>)> Callback);

	/// @brief Set language to get corresponding data from the server
	/// Note: Setting the language invalidates your local mod and game info cache.
	/// The next time this data is fetched, it will be in the new language.
	/// To get this localized content immediately, you must call
	/// 'FetchExternalUpdatesAsync' after changing the langauge.
	/// Without calling 'FetchExternalUpdatesAsync', mod and game info may be returned in a previous language.
	/// @param Locale language to set
	MODIOSDK_API void SetLanguage(Modio::Language Locale);

	/// @docpublic
	/// @brief Attempts to purchase a specified mod with an expected price. Purchasing a mod will add a subscription to
	/// it
	/// @param ModID The ID for the mod the user is trying to purchase
	/// @param ExpectedVirtualCurrencyPrice The price that was displayed to the user, if using the Virtual
	/// Currency-based purchasing flow.
	/// @param Callback Callback providing a status code and optional transaction confirmation if the purchase was
	/// successful. If the expected price that was displayed to the end user is no longer correct, for example if the
	/// price was increased or decreased by the mod creator since the price was displayed, an error will be returned. If
	/// this is the case you should refresh your user interface with the updated price and the user can retry the
	/// purchase at the new value.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @errorcategory MonetizationError|Problems during purchase transaction
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	MODIOSDK_API void PurchaseModAsync(
		Modio::ModID ModID, Modio::Optional<uint64_t> ExpectedVirtualCurrencyPrice,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)> Callback);

	/// @docpublic
	/// @brief Attempts to purchase a specified mod by consuming an existing, un-consumed entitlement that the user has
	/// from the current portal. Purchasing a mod will add a subscription to it. If the user does not have an existing
	/// entitlement, the purchase will fail. It is recommended to call GetAvailableUserEntitlementsAsync to query if the
	/// current user has an available entitlement prior to this call.
	/// @param ModID The ID for the mod the user is trying to purchase
	/// @param Portal-specific additional parameters
	/// @param Callback Callback providing a status code and optional transaction confirmation if the purchase was
	/// successful. If the user does not have an available entitlement of the correct type, the purchase will fail.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @errorcategory MonetizationError|Problems during purchase transaction
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error GenericError::BadParameter|The supplied mod ID is invalid
	/// @error MonetizationError::AccountLacksEntitlement|The user lacks a matching entitlement to make the purchase
	MODIOSDK_API void PurchaseModWithEntitlementAsync(
		Modio::ModID ModID, Modio::EntitlementParams Params,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)> Callback);

	/// @docpublic
	/// @brief Requests mod.io consume available entitlements for the current authenticated user purchased through the
	/// current portal.
	/// @param Params Additional parameters
	/// @param Callback Callback providing an error code indicating success or failure of the refresh operation
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @errorcategory MonetizationError|Problems during purchase transaction
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error MonetizationError::RetryEntitlements|Some entitlements could not be verified. Wait some time and try
	/// again.
	MODIOSDK_API void RefreshUserEntitlementsAsync(
		Modio::EntitlementParams Params,
		std::function<void(Modio::ErrorCode, Modio::Optional<EntitlementConsumptionStatusList>)> Callback);

	/// @docpublic
	/// @brief Retrieves a list of the unconsumed entitlements that the mod.io user has available via the current
	/// portal. Will not consume entitlements.
	/// @param Params Additional parameters specific to the portal in use
	/// @param Callback Callback providing an error code indicating success or failure of the retrieval operation
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @errorcategory MonetizationError|Problems during purchase transaction
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error UserDataError::InvalidUser|No authenticated user
	MODIOSDK_API void GetAvailableUserEntitlementsAsync(
		Modio::EntitlementParams Params,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)> Callback);

	/// @brief Fetches the updated mod.io wallet balance for the currently logged-in user
	/// @param Callback Callback providing an error code indicating success or failure, as well as a value
	/// indicating the updated balance
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @errorcategory MonetizationError|Problems during purchase transaction
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error UserDataError::InvalidUser|No authenticated user
	MODIOSDK_API
	void GetUserWalletBalanceAsync(std::function<void(Modio::ErrorCode, Modio::Optional<uint64_t>)> Callback);

	/// @brief Get the currently applied language
	/// @return Modio::Language currently set
	MODIOSDK_API Modio::Language GetLanguage();

	/// @docpublic
	/// @brief Fetches a list of all purchases that a User has made. Query the list using <<QueryUserPurchasedMods>>
	/// after a successful call.
	/// @param OnFetchDone Callback invoked with a status code for whether the fetch was successful
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @errorcategory MonetizationError|Problems during purchase transaction
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error UserDataError::InvalidUser|No authenticated user
	MODIOSDK_API void FetchUserPurchasesAsync(std::function<void(Modio::ErrorCode)> OnFetchDone);

	/// @docpublic
	/// @brief Fetches the locally cached view of the user's purchased mods, populated from
	/// <<RefreshUserPurchasesAsync>>
	/// @return std::map using Mod IDs as keys and ModInfo objects providing information about the mod that was
	/// purchased. mods
	MODIOSDK_API std::map<Modio::ModID, Modio::ModInfo> QueryUserPurchasedMods();

	/// @docpublic
	///	@brief Gets a UserDelegationToken that can be used by a backend serve for S2S functionality.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @errorcategory MonetizationError|Problems during purchase transaction
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error UserDataError::InvalidUser|No authenticated user
	MODIOSDK_API void GetUserDelegationTokenAsync(std::function<void(Modio::ErrorCode, std::string)> Callback);

	/// @docpublic
	/// @brief Initialize a Temp Mod Set, installing every specified mod
	/// given in the param if they are not already subbed.
	/// @param ModIds vector of ModID to install as temp mod
	///
	/// @return Modio::ErrorCode indicating if temp mod correctly started
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error ModManagementError::ModManagementDisabled|Mod Management need to be enabled
	MODIOSDK_API Modio::ErrorCode InitTempModSet(std::vector<Modio::ModID> ModIds);

	/// @docpublic
	/// @brief Add mods to an already created Temp Mod Session,
	/// install every temp mod given in the param if they already subbed or already in Temp Mod Set
	/// @param ModIds vector of ModID to install as temp mod
	///
	/// @return Modio::ErrorCode indicating if temp mod correctly started
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error ModManagementError::ModManagementDisabled|Mod Management need to be enabled
	/// @error ModManagementError::TempModSetNotInitialized|Temp Mod Set is not initialize, call InitTempModSet.
	MODIOSDK_API Modio::ErrorCode AddToTempModSet(std::vector<Modio::ModID> ModIds);

	/// @docpublic
	/// @brief Delete mods to an already created Temp Mod Session,
	/// delete every temp mods given in the param if they already subbed mod, it will not delete them.
	/// @param ModIds vector of ModID to install as temp mod
	///
	/// @return Modio::ErrorCode indicating if temp mod correctly started
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error ModManagementError::ModManagementDisabled|Mod Management need to be enabled
	/// @error ModManagementError::TempModSetNotInitialized|Temp Mod Set is not initialize, call InitTempModSet.
	MODIOSDK_API Modio::ErrorCode RemoveFromTempModSet(std::vector<Modio::ModID> ModIds);

	/// @docpublic
	/// @brief Close a Temp Mod Session,  delete all temp mods
	///
	/// @return Modio::ErrorCode indicating if temp mod correctly started
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error ModManagementError::ModManagementDisabled|Mod Management need to be enabled
	/// @error ModManagementError::TempModSetNotInitialized|Temp Mod Set is not initialize, call InitTempModSet.
	MODIOSDK_API Modio::ErrorCode CloseTempModSet();

	/// @docpublic
	/// @brief Fetches the temp mod added to Temp Mod Set
	///
	/// @return std::map using Mod IDs as keys and ModCollectionEntry objects providing information about the mods
	/// added to temp mod set
	MODIOSDK_API std::map<Modio::ModID, Modio::ModCollectionEntry> QueryTempModSet();

	/// @docpublic
	/// @brief Start a Metrics play session
	/// @param Params Modio::MetricsServiceParams object containing information of what and how to start a metrics
	/// session
	/// @param Callback Callback providing an error code indicating success or failure of the session start operation
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error MetricsError::SessionNotInitialized|Metrics session has not yet been initialized
	/// @error MetricsError::SessionIsActive|Metrics session is currently active and running
	/// @error GenericError::BadParameter|One or more values in the Metric Session Parameters are invalid
	/// @premiumfeature Metrics
	///	@experimental
	MODIOSDK_API void MetricsSessionStartAsync(Modio::MetricsSessionParams Params,
											   std::function<void(Modio::ErrorCode)> Callback);
	/// @docpublic
	/// @brief Sends a single heartbeat to the mod.io server to indicate a session is still active
	/// @param Callback Callback providing an error code indicating success or failure of the session heartbeat
	/// operation
	///
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error MetricsError::SessionNotInitialized|Metrics session has not yet been initialized
	/// @error MetricsError::SessionIsNotActive|Metrics session is not currently running.
	/// Call MetricsSessionStartAsync before attempting to sending a heartbeat.
	/// @premiumfeature Metrics
	///	@experimental
	MODIOSDK_API void MetricsSessionSendHeartbeatOnceAsync(std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Sends a constant heartbeat at a given interval to the mod.io server to indicate a session is still active
	/// @param IntervalSeconds The frequency in seconds to send a heartbeat to the mod.io server
	/// @param Callback Callback providing an error code indicating success or failure of the session heartbeat
	/// operation
	///
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error MetricsError::SessionNotInitialized|Metrics session has not yet been initialized
	/// @error MetricsError::SessionIsNotActive|Metrics session is not currently running.
	/// Call MetricsSessionStartAsync before attempting to sending a heartbeat.
	/// @premiumfeature Metrics
	///	@experimental
	MODIOSDK_API void MetricsSessionSendHeartbeatAtIntervalAsync(uint32_t IntervalSeconds,
																 std::function<void(Modio::ErrorCode)> Callback);
	/// @docpublic
	/// @brief Ends a Metrics play session
	/// @param Callback Callback providing an error code indicating success or failure of the session end operation
	///
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error MetricsError::SessionNotInitialized|Metrics session has not yet been initialized
	/// @error MetricsError::SessionIsNotActive|Metrics session is not currently running.
	/// Call MetricsSessionStartAsync before attempting to end a session.
	/// @premiumfeature Metrics
	///	@experimental
	MODIOSDK_API void MetricsSessionEndAsync(std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Initializes the SDK for in the context of a dedicated Server. Optionally gets/updates all mods listed in
	/// InitOptions.
	/// @param InitOptions Parameters to the function packed as a struct where all members needs to be
	/// initialized for the call to succeed
	/// @param OnInitComplete Callback for completion of the init process
	/// @errorcategory NetworkError|Couldn't connect to the mod.io servers
	/// @errorcategory FilesystemError|Couldn't create the user data or common data folders
	/// @errorcategory ConfigurationError|InitOptions contains an invalid value - inspect ec.value() to
	/// determine what was incorrect
	/// @error GenericError::SDKAlreadyInitialized|SDK already initialized
	/// @experimental
	MODIOSDK_API void InitializeModioServerAsync(Modio::ServerInitializeOptions InitOptions,
												 std::function<void(Modio::ErrorCode)> OnInitComplete);

	/// @docpublic
	/// @brief Gets and/or updates all of the indicated mods for this server, assuming you have provided a
	/// valid OAuth token
	/// @param Mods the mods to get or update in addition to those indicated in the initialization options
	/// @param Callback callback executed upon completion of all mods being validated and installed
	/// @experimental
	MODIOSDK_API void InstallOrUpdateServerModsAsync(std::vector<ModID> Mods,
													 std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Adds the provided Mods to the Server's list of client mods. Intended to be called on the
	/// server
	/// @param ModIDs a list of mods that the joining client has
	/// @param OnComplete callback upon completion of adding the mods to the Server's list. Contains a set of
	/// all registered client mods.
	/// @errorcategory NetworkError|Couldn't connect to the mod.io servers
	/// @experimental
	MODIOSDK_API void RegisterClientModsWithServerAsync(
		std::vector<Modio::ModID> ModIDs, std::function<void(Modio::ErrorCode, std::set<Modio::ModID>)> Callback);

	/// @docpublic
	/// @brief Clears the server's list of client mods.
	/// Intended only to be called when the Server safely flush its list of client mods, if you want to minimize
	/// the number of mods you send to clients/remove mods no longer needed.
	/// @experimental
	MODIOSDK_API void ClearRegisteredClientMods();

	/// @docpublic
	/// @brief Returns the full list of mods that have been registered with the server via AddClientModsToServerAsync
	/// @return A std::set of ModIDs representing the Mods registered with the server.
	/// @experimental
	MODIOSDK_API std::set<Modio::ModID> GetRegisteredClientMods();

	/// @docpublic
	/// @brief Provides a list of mod collections for the current game, that match the parameters specified in the
	/// filter
	/// @param Filter Modio::FilterParams object containing any filters that should be applied to the query
	/// @param Callback Callback invoked with a status code and an optional ModCollectionInfoList providing mod profiles
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void ListModCollectionsAsync(
		Modio::FilterParams Filter,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfoList>)> Callback);

	/// @docpublic
	/// @brief Fetches detailed information about the specified mod collection, including description and file metadata
	/// for the most recent release
	/// @param ModCollectionId Mod Collection ID of the mod colection to fetch data
	/// @param Callback Callback providing a status code and an optional Modio::ModCollectionInfo object with the mod
	/// collections's extended information
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod collection ID is invalid
	MODIOSDK_API void GetModCollectionInfoAsync(
		Modio::ModCollectionID ModCollectionId,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfo>)> Callback);

	/// @docpublic
	/// @brief Get a list of Mods contained within the specified mod collection
	/// @param ModCollectionId Mod Collection ID of the mod colection to fetch data
	/// @param Callback Callback providing a status code and an optional Modio::ModInfoList object with the mods
	/// contained within the mod collections
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod collection ID is invalid
	MODIOSDK_API void GetModCollectionModsAsync(
		Modio::ModCollectionID ModCollectionId,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)> Callback);

	/// @docpublic
	/// @brief Submits a rating for a mod collection on behalf of the currently authenticated user.
	/// @param ModCollectionId The mod collection to submit a rating for
	/// @param Rating The rating to submit \r\n NOTE: To clear a rating for a mod collection, submit a rating of
	/// Rating::Neutral.
	/// @param Callback Callback providing a status code to indicate the rating was submitted successfully
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @requires authenticated-user
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod collection could not be found
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod collection ID is invalid
	MODIOSDK_API void SubmitModCollectionRatingAsync(Modio::ModCollectionID ModCollectionId, Modio::Rating Rating,
													 std::function<void(Modio::ErrorCode)> Callback);

	/// @docpublic
	/// @brief Sends a request to the mod.io server to add the specified mod collection to the user's list of
	/// subscriptions.
	/// NOTE: Unlike SubscribeToModAsync, SubscribeToModeCollectionAsync does not automatically trigger installation of
	/// new subscriptions arising from a successful request. Call FetchExternalUpdatesAsync after this function succeeds
	/// in order to initiate installation.
	/// @param ModCollectionToSubscribeTo Mod Collection ID of the mod collection requiring a subscription.
	/// @param IncludeDependencies If this mod has any dependencies, all of those will also be subscribed to.
	/// @param OnSubscribeComplete Callback invoked when the subscription request is completed.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @requires management-enabled
	/// @requires mod-not-pending-uninstall
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error ModManagementError::ModBeingProcessed|Specified mod is pending uninstall. Wait until the uninstall
	/// process is complete before subscribing again.
	/// @error GenericError::BadParameter|The supplied mod collection ID is invalid
	MODIOSDK_API void SubscribeToModCollectionAsync(Modio::ModCollectionID ModCollectionToSubscribeTo,
													std::function<void(Modio::ErrorCode)> OnSubscribeComplete);

	/// @docpublic
	/// @brief Sends a request to the mod.io server to remove the specified mod collection from the user's list of
	/// subscriptions. If no other local users are subscribed to the mods in the specified collection, this function
	/// will also mark the mods in the collection for uninstallation by the SDK.
	/// @param ModCollectionToUnsubscribeFrom Mod Collection ID of the mod collection requiring unsubscription.
	/// @param OnUnsubscribeComplete Callback invoked when the unsubscription request is completed.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @requires management-enabled
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod collection does not exist or was deleted
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod collection ID is invalid
	/// \todo  Is the local uninstallation comment in brief still valid?
	MODIOSDK_API void UnsubscribeFromModCollectionAsync(Modio::ModCollectionID ModCollectionToUnsubscribeFrom,
														std::function<void(Modio::ErrorCode)> OnUnsubscribeComplete);

	/// @docpublic
	/// @brief Sends a request to the mod.io server to add the specified mod collection to the user's list of
	/// followed collections.
	/// @param ModCollectionToFollow Mod Collection ID of the mod collection requiring a subscription.
	/// @param OnFollowComplete Callback invoked when the follow request is completed.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @requires management-enabled
	/// @requires mod-not-pending-uninstall
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod does not exist or was deleted
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error ModManagementError::ModBeingProcessed|Specified mod is pending uninstall. Wait until the uninstall
	/// process is complete before subscribing again.
	/// @error GenericError::BadParameter|The supplied mod collection ID is invalid
	MODIOSDK_API void FollowModCollectionAsync(
		Modio::ModCollectionID ModCollectionToFollow,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfo>)> OnFollowComplete);

	/// @docpublic
	/// @brief Sends a request to the mod.io server to remove the specified mod collection from the user's list of
	/// followed collections.
	/// @param ModCollectionToUnfollow Mod Collection ID of the mod collection to unfollow.
	/// @param OnUnsubscribeComplete Callback invoked when the unfollow request is completed.
	/// @requires initialized-sdk
	/// @requires authenticated-user
	/// @requires no-rate-limiting
	/// @requires management-enabled
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod collection does not exist or was deleted
	/// @error UserDataError::InvalidUser|No authenticated user
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod collection ID is invalid
	MODIOSDK_API void UnfollowModCollectionAsync(Modio::ModCollectionID ModCollectionToUnfollow,
												 std::function<void(Modio::ErrorCode)> OnUnfollowComplete);
	/// @docpublic
	/// @brief Provides a list of followed mod collections for the current game, that match the parameters specified
	/// in the filter.
	/// @param Filter Modio::FilterParams object containing any filters that should be applied to the query
	/// @param Callback Callback invoked with a status code and an optional ModCollectionInfoList providing mod
	/// collection profiles
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	MODIOSDK_API void ListUserFollowedModCollectionsAsync(
		Modio::FilterParams Filter,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfoList>)> Callback);

	/// @docpublic
	/// @brief Downloads the logo for the specified mod collection. Will use existing file if it is already present on
	/// disk
	/// @param ModId Mod Collection ID for use in logo retrieval
	/// @param LogoSize Parameter indicating the size of logo that's required
	/// @param Callback Callback providing a status code and an optional path object pointing to the location of the
	/// downloaded image
	/// @requires initialized-sdk
	/// @requires no-rate-limiting
	/// @errorcategory NetworkError|Couldn't connect to mod.io servers
	/// @error GenericError::SDKNotInitialized|SDK not initialized
	/// @errorcategory EntityNotFoundError|Specified mod collection media does not exist or was deleted
	/// @error FilesystemError::InsufficientSpace|Not enough space for the file
	/// @error HttpError::RateLimited|Too many frequent calls to the API. Wait some time and try again.
	/// @error GenericError::BadParameter|The supplied mod collection ID is invalid
	MODIOSDK_API void GetModCollectionMediaAsync(
		Modio::ModCollectionID CollectionId, Modio::LogoSize LogoSize,
		std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback);

	/// @docpublic
	/// @brief Downloads the creator avatar for a specified mod collection. Will use existing file if it is already
	/// present on disk and not outdated
	/// @param CollectionId ID of the mod collection the creator avatar will be retrieved for
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
	/// @error GenericError::BadParameter|The supplied mod collection ID is invalid
	MODIOSDK_API void GetModCollectionMediaAsync(
		Modio::ModCollectionID CollectionId, Modio::AvatarSize AvatarSize,
		std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback);

	/// @docpublic
} // namespace Modio

// Implementation headers

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/impl/SDKCore.ipp"
	#include "modio/impl/SDKMetrics.ipp"
	#include "modio/impl/SDKModCollections.ipp"
	#include "modio/impl/SDKModManagement.ipp"
	#include "modio/impl/SDKModMetadata.ipp"
	#include "modio/impl/SDKMonetization.ipp"
	#include "modio/impl/SDKUserData.ipp"
#endif

#include "modio/detail/ModioUndefs.h"
