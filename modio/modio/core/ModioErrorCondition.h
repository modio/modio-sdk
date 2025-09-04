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

#include "ModioErrorCondition.generated.h"

UENUM(BlueprintType)
enum class EModioErrorCondition: uint8
{
	NoError = 0,
	NetworkError = 2 UMETA(ToolTip="When this condition is true, the error code represents a connection or HTTP error between the client and the mod.io server.") ,
	ConfigurationError = 3 UMETA(ToolTip="When this condition is true, the error code indicates the SDK's configuration is not valid - the game ID or API key are incorrect or the game has been deleted.") ,
	InvalidArgsError = 4 UMETA(ToolTip="When this condition is true, the error code indicates the arguments passed to the function have failed validation or were otherwise invalid.") ,
	FilesystemError = 5 UMETA(ToolTip="When this condition is true, the error code indicates a permission or IO error when accessing local filesystem data.") ,
	InternalError = 6 UMETA(ToolTip="When this condition is true, the error code represents an internal SDK error - please inform mod.io of the error code value.") ,
	ApiErrorRefSuccess = 7 UMETA(ToolTip="When this condition is true, the error ref returned by the API indicates an implicit success because the operation has already been performed (ie a no-op is success).") ,
	EntityNotFoundError = 12 UMETA(ToolTip="When this condition is true, the error code indicates that a specified game, mod, user, media file or mod file was not found.") ,
	UserTermsOfUseError = 13 UMETA(ToolTip="When this condition is true, the error code indicates that the user has not yet accepted the mod.io Terms of Use.") ,
	SubmitReportError = 14 UMETA(ToolTip="When this condition is true, the error code indicates that a report for the specified content could not be submitted.") ,
	UserNotAuthenticatedError = 15 UMETA(ToolTip="When this condition is true, the error code indicates that a user is not authenticated.") ,
	SDKNotInitialized = 16 UMETA(ToolTip="When this condition is true, the error code indicates that the SDK has not been initialized.") ,
	UserAlreadyAuthenticatedError = 17 UMETA(ToolTip="When this condition is true, the error code indicates that the user is already authenticated.") ,
	SystemError = 18 UMETA(ToolTip="When this condition is true, the error code indicates that a low-level system error occurred outside of mod.io SDK control.") ,
	OperationCanceled = 19 UMETA(ToolTip="When this condition is true, the error code indicates that the asynchronous operation was cancelled before it completed.") ,
	ModManagementDisabled = 20 UMETA(ToolTip="When this condition is true, the error code indicates that Mod Management has not been enabled.") ,
	RateLimited = 21 UMETA(ToolTip="Too many requests made to the mod.io API within the rate-limiting window. Please wait and try again.") ,
	ModBeingProcessed = 22 UMETA(ToolTip="The specified mod's files are currently being updated by the SDK. Please try again later.") ,
	InsufficientSpace = 23 UMETA(ToolTip="There is insufficient space to install the mod. Please free up space and try again.") ,
	SDKAlreadyInitialized = 24 UMETA(ToolTip="When this condition is true, the error code indicates that the SDK has already been initialized.") ,
	ModManagementAlreadyEnabled = 25 UMETA(ToolTip="When this condition is true, the error code indicates that Mod Management has already been enabled.") ,
	InsufficientPermissions = 26 UMETA(ToolTip="When this condition is true, the error code indicates that the current user does not have the required permissions for this operation.") ,
	EmailLoginCodeInvalid = 27 UMETA(ToolTip="The email login code is incorrect, has expired, or has already been used.") ,
	AlreadySubscribed = 28 UMETA(ToolTip="The specified mod is already subscribed to.") ,
	InstallOrUpdateCancelled = 29 UMETA(ToolTip="The current mod installation or update was cancelled.") ,
	UploadCancelled = 30 UMETA(ToolTip="The current modfile upload was cancelled.") ,
	TempModSetNotInitialized = 31 UMETA(ToolTip="TempModSet need to be initialized first, call InitTempModSet.") ,
	MonetizationOperationError = 32 UMETA(ToolTip="An error occurred while performing a monetization operation.") ,
	PaymentTransactionFailed = 33 UMETA(ToolTip="The transaction requires a payment but it could not be fulfilled. Please retry with funds on the wallet") ,
	IncorrectPrice = 34 UMETA(ToolTip="The display price for the mod is out-of-date or incorrect. Please retry with the correct display price.") ,
	ItemAlreadyOwned = 35 UMETA(ToolTip="The authenticated user already has acquired this item") ,
	ParentalControlRestrictions = 36 UMETA(ToolTip="Parental control restrictions prevent this account from accessing UGC.") ,
	MetricsSessionNotInitialized = 37 UMETA(ToolTip="Metrics session has not yet been initialized. Ensure that you have a metrics secret key set for your project.") ,
	MetricsSessionAlreadyInitialized = 38 UMETA(ToolTip="Metrics session has already been been initialized.") ,
	MetricsSessionIsActive = 39 UMETA(ToolTip="Metrics session has been started.") ,
	MetricsSessionIsNotActive = 40 UMETA(ToolTip="Metrics session has not been started. Please call MetricsSessionStartAsync.") ,
	MetricsSessionHasNoMods = 41 UMETA(ToolTip="No mods have been added to the session.") ,
	PremiumFeatureNotAvailable = 42 UMETA(ToolTip="This premium feature is not available for your project.") ,
	EmailExchangeCodeAlreadyRedeemed = 43 UMETA(ToolTip="The email security code has already been redeemed.") ,
	ModDependenciesNotAllowed = 44 UMETA(ToolTip="Cannot add a dependency because the target mod has not opted in to dependencies.") ,
	ModCannotAddDependencyMonetized = 45 UMETA(ToolTip="Cannot add a dependency because the mod is monetized.") ,
	ModIsDependency = 46 UMETA(ToolTip="Cannot turn off mod dependencies when the mod is currently a dependency for other mods.") ,
	ModCannotAllowDependencyMonetized = 47 UMETA(ToolTip="This mod cannot allow dependencies because it is monetized.") ,
	ModCannotDeleteDependency = 48 UMETA(ToolTip="This mod is a dependency of other mods and cannot be deleted.") ,
	RequestInProgress = 49 UMETA(ToolTip="The asynchronous operation is already running. Please wait for it to complete before calling it again") ,
	InvalidZipFile = 50 UMETA(ToolTip="The zip file submitted is invalid.") ,
};
