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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


#if defined(__has_attribute)
#define MODIO_HAS_ATTRIBUTE(attribute) __has_attribute(attribute)
#else
#define MODIO_HAS_ATTRIBUTE(attribute) (0)
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
    #define MODIO_EXPORT   __declspec(dllexport) 
    #define MODIO_IMPORT   __declspec(dllimport) 
#else
#if MODIO_HAS_ATTRIBUTE(visibility) 
    #define MODIO_EXPORT __attribute__((__visibility__("default")))
#endif
    #define MODIO_IMPORT    extern
#endif

#ifdef MODIO_BUILD_DLL
    #define MODIODLL_EXPORT MODIO_EXPORT
#else
    #define MODIODLL_EXPORT MODIO_IMPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum EModioLanguage
{
    EModioLanguage_English,
    EModioLanguage_Bulgarian,
    EModioLanguage_French,
    EModioLanguage_German,
    EModioLanguage_Italian,
    EModioLanguage_Polish,
    EModioLanguage_Portuguese,
    EModioLanguage_Hungarian,
    EModioLanguage_Japanese,
    EModioLanguage_Korean,
    EModioLanguage_Russian,
    EModioLanguage_Spanish,
    EModioLanguage_Thai,
    EModioLanguage_ChineseSimplified,
    EModioLanguage_ChineseTraditional
} EModioLanguage; 

typedef enum EModioAuthenticationProvider
{
    EModioAuthenticationProvider_XBoxLive,
    EModioAuthenticationProvider_Steam,
    EModioAuthenticationProvider_GoG,
    EModioAuthenticationProvider_Itch,
    EModioAuthenticationProvider_Switch,
    EModioAuthenticationProvider_Discord,
    EModioAuthenticationProvider_PSN,
    EModioAuthenticationProvider_Oculus,
    EModioAuthenticationProvider_Epic,
    EModioAuthenticationProvider_OpenID,
    EModioAuthenticationProvider_Apple,
    EModioAuthenticationProvider_GoogleIDToken,
    EModioAuthenticationProvider_GoogleServerSideToken
} EModioAuthenticationProvider; 

typedef enum EModioSortFieldType
{
    EModioSortFieldType_ID,
    EModioSortFieldType_DownloadsToday,
    EModioSortFieldType_SubscriberCount,
    EModioSortFieldType_Rating,
    EModioSortFieldType_DateMarkedLive,
    EModioSortFieldType_DateUpdated,
    EModioSortFieldType_DownloadsTotal,
    EModioSortFieldType_Alphabetical
} EModioSortFieldType; 

typedef enum EModioRevenueFilterType
{
    EModioRevenueFilterType_Free,
    EModioRevenueFilterType_Paid,
    EModioRevenueFilterType_FreeAndPaid
} EModioRevenueFilterType; 

typedef enum EModioSortDirection
{
    EModioSortDirection_Ascending,
    EModioSortDirection_Descending
} EModioSortDirection; 

typedef enum EModioLogLevel
{
    EModioLogLevel_Trace = 0,
    EModioLogLevel_Detailed = 1,
    EModioLogLevel_Info = 2,
    EModioLogLevel_Warning = 3,
    EModioLogLevel_Error = 4
} EModioLogLevel; 

typedef enum EModioMaturityOption
{
    EModioMaturityOption_None = 0,
    EModioMaturityOption_Alcohol = 1,
    EModioMaturityOption_Drugs = 2,
    EModioMaturityOption_Violence = 4,
    EModioMaturityOption_Explicit = 8
} EModioMaturityOption; 

typedef int32_t CModioMaturityProfile;
inline bool ModioMaturityProfileHasFlag(CModioMaturityProfile ValueToCheck, EModioMaturityOption Flag)
{
    return (ValueToCheck & Flag) != 0;
}
inline CModioMaturityProfile ModioMaturityProfileSetFlag(CModioMaturityProfile Value, EModioMaturityOption Flag)
{
    return Value | Flag;
}
inline CModioMaturityProfile ModioMaturityProfileClearFlag(CModioMaturityProfile Value, EModioMaturityOption Flag)
{
    return Value & (~Flag);
}
inline CModioMaturityProfile ModioMaturityProfileToggleFlag(CModioMaturityProfile Value, EModioMaturityOption Flag)
{
    return Value ^ Flag;
}

typedef struct COptionalModioMaturityProfile
{
    CModioMaturityProfile Value;
    bool HasValue;
} COptionalModioMaturityProfile;

typedef enum EModioPortal
{
    EModioPortal_None,
    EModioPortal_Apple,
    EModioPortal_EpicGamesStore,
    EModioPortal_GOG,
    EModioPortal_Google,
    EModioPortal_Itchio,
    EModioPortal_Nintendo,
    EModioPortal_PSN,
    EModioPortal_Steam,
    EModioPortal_XboxLive
} EModioPortal; 

typedef enum EModioEnvironment
{
    EModioEnvironment_Test,
    EModioEnvironment_Live
} EModioEnvironment; 

typedef enum EModioErrorCondition
{
    EModioErrorCondition_ModioServiceError = 1,
    EModioErrorCondition_NetworkError = 2,
    EModioErrorCondition_ConfigurationError = 3,
    EModioErrorCondition_InvalidArgsError = 4,
    EModioErrorCondition_FilesystemError = 5,
    EModioErrorCondition_InternalError = 6,
    EModioErrorCondition_ApiErrorRefSuccess = 7,
    EModioErrorCondition_ModInstallRetryableError = 8,
    EModioErrorCondition_ModInstallDeferredError = 9,
    EModioErrorCondition_ModDeleteDeferredError = 10,
    EModioErrorCondition_ModInstallUnrecoverableError = 11,
    EModioErrorCondition_EntityNotFoundError = 12,
    EModioErrorCondition_UserTermsOfUseError = 13,
    EModioErrorCondition_SubmitReportError = 14,
    EModioErrorCondition_UserNotAuthenticatedError = 15,
    EModioErrorCondition_SDKNotInitialized = 16,
    EModioErrorCondition_UserAlreadyAuthenticatedError = 17,
    EModioErrorCondition_SystemError = 18,
    EModioErrorCondition_OperationCanceled = 19,
    EModioErrorCondition_ModManagementDisabled = 20,
    EModioErrorCondition_RateLimited = 21,
    EModioErrorCondition_ModBeingProcessed = 22,
    EModioErrorCondition_InsufficientSpace = 23,
    EModioErrorCondition_SDKAlreadyInitialized = 24,
    EModioErrorCondition_ModManagementAlreadyEnabled = 25,
    EModioErrorCondition_InsufficientPermissions = 26,
    EModioErrorCondition_EmailLoginCodeInvalid = 27,
    EModioErrorCondition_AlreadySubscribed = 28,
    EModioErrorCondition_InstallOrUpdateCancelled = 29,
    EModioErrorCondition_UploadCancelled = 30,
    EModioErrorCondition_TempModSetNotInitialized = 31,
    EModioErrorCondition_MonetizationOperationError = 32,
    EModioErrorCondition_PaymentTransactionFailed = 33,
    EModioErrorCondition_IncorrectPrice = 34,
    EModioErrorCondition_ItemAlreadyOwned = 35
} EModioErrorCondition; 

typedef enum EModioVirusScanStatus
{
    EModioVirusScanStatus_NotScanned = 0,
    EModioVirusScanStatus_ScanComplete = 1,
    EModioVirusScanStatus_InProgress = 2,
    EModioVirusScanStatus_TooLargeToScan = 3,
    EModioVirusScanStatus_FileNotFound = 4,
    EModioVirusScanStatus_ErrorScanning = 5
} EModioVirusScanStatus; 

typedef enum EModioVirusStatus
{
    EModioVirusStatus_NoThreat = 0,
    EModioVirusStatus_Malicious = 1,
    EModioVirusStatus_PotentiallyHarmful = 2
} EModioVirusStatus; 

typedef enum EModioModManagementEventType
{
    EModioModManagementEventType_BeginInstall,
    EModioModManagementEventType_Installed,
    EModioModManagementEventType_BeginUninstall,
    EModioModManagementEventType_Uninstalled,
    EModioModManagementEventType_BeginUpdate,
    EModioModManagementEventType_Updated,
    EModioModManagementEventType_BeginUpload,
    EModioModManagementEventType_Uploaded
} EModioModManagementEventType; 

typedef enum EModioModServerSideStatus
{
    EModioModServerSideStatus_NotAccepted = 0,
    EModioModServerSideStatus_Accepted = 1,
    EModioModServerSideStatus_Deleted = 3
} EModioModServerSideStatus; 

typedef struct COptionalModServerSideStatus
{
    EModioModServerSideStatus Value;
    bool HasValue;
} COptionalModServerSideStatus;

typedef enum EModioModState
{
    EModioModState_InstallationPending,
    EModioModState_Installed,
    EModioModState_UpdatePending,
    EModioModState_Downloading,
    EModioModState_Extracting,
    EModioModState_UninstallPending
} EModioModState; 

typedef enum EModioAvatarSize
{
    EModioAvatarSize_Original,
    EModioAvatarSize_Thumb50,
    EModioAvatarSize_Thumb100
} EModioAvatarSize; 

typedef enum EAuthTokenState
{
    EAuthTokenState_Valid,
    EAuthTokenState_Expired,
    EAuthTokenState_Invalid
} EAuthTokenState; 

typedef enum EModioModfilePlatform
{
    EModioModfilePlatform_Windows,
    EModioModfilePlatform_Mac,
    EModioModfilePlatform_Linux,
    EModioModfilePlatform_Android,
    EModioModfilePlatform_iOS,
    EModioModfilePlatform_XboxOne,
    EModioModfilePlatform_XboxSeriesX,
    EModioModfilePlatform_PS4,
    EModioModfilePlatform_PS5,
    EModioModfilePlatform_Switch,
    EModioModfilePlatform_Oculus,
    EModioModfilePlatform_Source
} EModioModfilePlatform; 

typedef enum EModioLogoSize
{
    EModioLogoSize_Original,
    EModioLogoSize_Thumb320,
    EModioLogoSize_Thumb640,
    EModioLogoSize_Thumb1280
} EModioLogoSize; 

typedef enum EModioGallerySize
{
    EModioGallerySize_Original,
    EModioGallerySize_Thumb320,
    EModioGallerySize_Thubm1280
} EModioGallerySize; 

typedef enum EModioRating
{
    EModioRating_Neutral = 0,
    EModioRating_Positive = 1,
    EModioRating_Negative = -1
} EModioRating; 

typedef enum EModioEntitlementConsumptionState
{
    EModioEntitlementConsumptionState_Failed = 0,
    EModioEntitlementConsumptionState_Pending = 1,
    EModioEntitlementConsumptionState_Fulfilled = 2,
    EModioEntitlementConsumptionState_ConsumeLimitExceeded = 3
} EModioEntitlementConsumptionState; 

typedef enum EModioEntitlementType
{
    EModioEntitlementType_VirtualCurrency = 0
} EModioEntitlementType; 

typedef enum EModioReportType
{
    EModioReportType_Generic = 0,
    EModioReportType_DMCA = 1,
    EModioReportType_NotWorking = 2,
    EModioReportType_RudeContent = 3,
    EModioReportType_IllegalContent = 4,
    EModioReportType_StolenContent = 5,
    EModioReportType_FalseInformation = 6,
    EModioReportType_Other = 7
} EModioReportType; 

typedef enum EModioChangeType
{
    EModioChangeType_Added = 0,
    EModioChangeType_Removed = 1,
    EModioChangeType_Updated = 2
} EModioChangeType; 

typedef enum EModioObjectVisibility
{
    EModioObjectVisibility_Hidden = 0,
    EModioObjectVisibility_Public = 1
} EModioObjectVisibility; 

typedef struct COptionalObjectVisibility
{
    EModioObjectVisibility Value;
    bool HasValue;
} COptionalObjectVisibility;

typedef enum EModioGameCommunityOption
{
    EModioGameCommunityOption_None = 0,
    EModioGameCommunityOption_EnableComments = 1,
    EModioGameCommunityOption_EnableGuides = 2,
    EModioGameCommunityOption_PinOnHomepage = 4,
    EModioGameCommunityOption_ShowOnHomepage = 8,
    EModioGameCommunityOption_ShowMoreOnHomepage = 16,
    EModioGameCommunityOption_AllowChangeStatus = 32,
    EModioGameCommunityOption_EnablePreviews = 64,
    EModioGameCommunityOption_AllowPreviewShareURL = 128,
    EModioGameCommunityOption_AllowNegativeRatings = 256,
    EModioGameCommunityOption_AllowModsToBeEditedOnWeb = 512
} EModioGameCommunityOption; 

typedef int32_t CModioGameCommunityOptionsFlags;
inline bool ModioGameCommunityOptionsFlagsHasFlag(CModioGameCommunityOptionsFlags ValueToCheck, EModioGameCommunityOption Flag)
{
    return (ValueToCheck & Flag) != 0;
}
inline CModioGameCommunityOptionsFlags ModioGameCommunityOptionsFlagsSetFlag(CModioGameCommunityOptionsFlags Value, EModioGameCommunityOption Flag)
{
    return Value | Flag;
}
inline CModioGameCommunityOptionsFlags ModioGameCommunityOptionsFlagsClearFlag(CModioGameCommunityOptionsFlags Value, EModioGameCommunityOption Flag)
{
    return Value & (~Flag);
}
inline CModioGameCommunityOptionsFlags ModioGameCommunityOptionsFlagsToggleFlag(CModioGameCommunityOptionsFlags Value, EModioGameCommunityOption Flag)
{
    return Value ^ Flag;
}

typedef struct COptionalModioGameCommunityOptionsFlags
{
    CModioGameCommunityOptionsFlags Value;
    bool HasValue;
} COptionalModioGameCommunityOptionsFlags;

typedef enum EModioGameMonetizationOption
{
    EModioGameMonetizationOption_None = 0,
    EModioGameMonetizationOption_Monetization = 1,
    EModioGameMonetizationOption_Marketplace = 2,
    EModioGameMonetizationOption_PartnerProgram = 4
} EModioGameMonetizationOption; 

typedef int32_t CModioGameMonetization;
inline bool ModioGameMonetizationHasFlag(CModioGameMonetization ValueToCheck, EModioGameMonetizationOption Flag)
{
    return (ValueToCheck & Flag) != 0;
}
inline CModioGameMonetization ModioGameMonetizationSetFlag(CModioGameMonetization Value, EModioGameMonetizationOption Flag)
{
    return Value | Flag;
}
inline CModioGameMonetization ModioGameMonetizationClearFlag(CModioGameMonetization Value, EModioGameMonetizationOption Flag)
{
    return Value & (~Flag);
}
inline CModioGameMonetization ModioGameMonetizationToggleFlag(CModioGameMonetization Value, EModioGameMonetizationOption Flag)
{
    return Value ^ Flag;
}

typedef struct COptionalBool
{
	bool Value;
	bool HasValue;
} COptionalBool;

typedef struct COptionalUInt64
{
	uint64_t Value;
	bool HasValue;
} COptionalUInt64;

typedef struct CModioString CModioString;
typedef struct CModioTag CModioTag;
typedef struct CModioModManagementEvent CModioModManagementEvent;
typedef struct CModioModCreationHandle CModioModCreationHandle;
typedef struct CModioLink CModioLink;
typedef struct CModioEditModParams CModioEditModParams;
typedef struct CModioIcon CModioIcon;
typedef struct CModioLogo CModioLogo;
typedef struct CModioHeaderImage CModioHeaderImage;
typedef struct CModioOtherUrl CModioOtherUrl;
typedef struct CModioOtherUrlList CModioOtherUrlList;
typedef struct CModioTheme CModioTheme;
typedef struct CModioGameStats CModioGameStats;
typedef struct CModioTermsOfUse CModioTermsOfUse;
typedef struct CModioModTagList CModioModTagList;
typedef struct CModioModStats CModioModStats;
typedef struct CModioGameInfo CModioGameInfo;
typedef struct CModioModLogo CModioModLogo;
typedef struct CModioKVP CModioKVP;
typedef struct CModioMetadataKVP CModioMetadataKVP;
typedef struct CModioFileInfo CModioFileInfo;
typedef struct CModioUser CModioUser;
typedef struct CModioFilterParams CModioFilterParams;
typedef struct CModioAvatar CModioAvatar;
typedef struct CModioFieldError CModioFieldError;
typedef struct CModioValidationError CModioValidationError;
typedef struct CModioValidationErrorList CModioValidationErrorList;
typedef struct CModioModProgressInfo CModioModProgressInfo;
typedef struct CModioStringList CModioStringList;
typedef struct CModioStringMap CModioStringMap;
typedef struct CModioStringMapIterator CModioStringMapIterator;
typedef struct CModioAuthenticationParams CModioAuthenticationParams;
typedef struct CModioModInfoList CModioModInfoList;
typedef struct CModioUserList CModioUserList;
typedef struct CModioGamePlatform CModioGamePlatform;
typedef struct CModioModTagInfo CModioModTagInfo;
typedef struct CModioModTagOptions CModioModTagOptions;
typedef struct CModioModTagInfoList CModioModTagInfoList;
typedef struct CModioModCollectionMap CModioModCollectionMap;
typedef struct CModioModCollectionMapIterator CModioModCollectionMapIterator;
typedef struct CModioModDependency CModioModDependency;
typedef struct CModioModDependencyList CModioModDependencyList;
typedef struct CModioModCollectionEntry CModioModCollectionEntry;
typedef struct CModioImageList CModioImageList;
typedef struct CModioErrorCode CModioErrorCode;
typedef struct CModioAPIKey CModioAPIKey;
typedef struct CModioModInfo CModioModInfo;
typedef struct CModioImage CModioImage;
typedef struct CModioCreateModParams CModioCreateModParams;
typedef struct CModioPlatformList CModioPlatformList;
typedef struct CModioGamePlatformList CModioGamePlatformList;
typedef struct CModioCreateModFileParams CModioCreateModFileParams;
typedef struct CModioAuthToken CModioAuthToken;
typedef struct CModioModID CModioModID;
typedef struct CModioModIDList CModioModIDList;
typedef struct CModioUserID CModioUserID;
typedef struct CModioUserIDList CModioUserIDList;
typedef struct CModioFileMetadataID CModioFileMetadataID;
typedef struct CModioInitializeOptions CModioInitializeOptions;
typedef struct CModioGameID CModioGameID;
typedef struct CModioReportParams CModioReportParams;
typedef struct CModioTransactionRecord CModioTransactionRecord;
typedef struct CModioEntitlementParams CModioEntitlementParams;
typedef struct CModioChangeMap CModioChangeMap;
typedef struct CModioChangeMapIterator CModioChangeMapIterator;
typedef struct CModioEntitlementConsumptionStatus CModioEntitlementConsumptionStatus;
typedef struct CModioEntitlementConsumptionStatusList CModioEntitlementConsumptionStatusList;

MODIODLL_EXPORT CModioString* CreateModioString(const char* Data, size_t Length);
MODIODLL_EXPORT void ReleaseModioString(CModioString* Item);
MODIODLL_EXPORT CModioString* CopyModioString(const CModioString* Item);

MODIODLL_EXPORT const char* GetModioStringData(const CModioString* ModioString);
MODIODLL_EXPORT size_t GetModioStringLength(const CModioString* ModioString);

MODIODLL_EXPORT CModioTag* CreateModioTag();
MODIODLL_EXPORT void ReleaseModioTag(CModioTag* Item);
MODIODLL_EXPORT CModioTag* CopyModioTag(const CModioTag* Item);

MODIODLL_EXPORT CModioString* GetModioTagTag(const CModioTag* ModioTag);
MODIODLL_EXPORT CModioString* GetModioTagTagLocalized(const CModioTag* ModioTag);
MODIODLL_EXPORT void SetModioTagTag(CModioTag* Item, CModioString const* Tag);
MODIODLL_EXPORT void SetModioTagTagLocalized(CModioTag* Item, CModioString const* TagLocalized);

MODIODLL_EXPORT CModioModManagementEvent* CreateModioModManagementEvent();
MODIODLL_EXPORT void ReleaseModioModManagementEvent(CModioModManagementEvent* Item);
MODIODLL_EXPORT CModioModManagementEvent* CopyModioModManagementEvent(const CModioModManagementEvent* Item);

MODIODLL_EXPORT CModioModID* GetModioModManagementEventModID(const CModioModManagementEvent* ModManagementEvent);
MODIODLL_EXPORT EModioModManagementEventType GetModioModManagementEventEventType(const CModioModManagementEvent* ModManagementEvent);
MODIODLL_EXPORT CModioErrorCode* GetModioModManagementEventStatus(const CModioModManagementEvent* ModManagementEvent);
MODIODLL_EXPORT void SetModioModManagementEventModID(CModioModManagementEvent* Item, CModioModID const* ModID);
MODIODLL_EXPORT void SetModioModManagementEventEventType(CModioModManagementEvent* Item, EModioModManagementEventType EventType);
MODIODLL_EXPORT void SetModioModManagementEventStatus(CModioModManagementEvent* Item, CModioErrorCode const* Status);

MODIODLL_EXPORT CModioModCreationHandle* CreateModioModCreationHandle();
MODIODLL_EXPORT void ReleaseModioModCreationHandle(CModioModCreationHandle* Item);
MODIODLL_EXPORT CModioModCreationHandle* CopyModioModCreationHandle(const CModioModCreationHandle* Item);
MODIODLL_EXPORT bool AreEqualModioModCreationHandle(const CModioModCreationHandle* First, const CModioModCreationHandle* Second);


MODIODLL_EXPORT CModioLink* CreateModioLink();
MODIODLL_EXPORT void ReleaseModioLink(CModioLink* Item);
MODIODLL_EXPORT CModioLink* CopyModioLink(const CModioLink* Item);

MODIODLL_EXPORT CModioString* GetModioLinkText(const CModioLink* ModioLink);
MODIODLL_EXPORT CModioString* GetModioLinkURL(const CModioLink* ModioLink);
MODIODLL_EXPORT bool GetModioLinkRequired(const CModioLink* ModioLink);
MODIODLL_EXPORT void SetModioLinkText(CModioLink* Item, CModioString const* Text);
MODIODLL_EXPORT void SetModioLinkURL(CModioLink* Item, CModioString const* URL);
MODIODLL_EXPORT void SetModioLinkRequired(CModioLink* Item, bool Required);

MODIODLL_EXPORT CModioEditModParams* CreateModioEditModParams();
MODIODLL_EXPORT void ReleaseModioEditModParams(CModioEditModParams* Item);
MODIODLL_EXPORT CModioEditModParams* CopyModioEditModParams(const CModioEditModParams* Item);

MODIODLL_EXPORT CModioString* GetModioEditModParamsName(const CModioEditModParams* ModioEditModParams);
MODIODLL_EXPORT CModioString* GetModioEditModParamsSummary(const CModioEditModParams* ModioEditModParams);
MODIODLL_EXPORT CModioString* GetModioEditModParamsNamePath(const CModioEditModParams* ModioEditModParams);
MODIODLL_EXPORT COptionalObjectVisibility GetModioEditModParamsVisibility(const CModioEditModParams* ModioEditModParams);
MODIODLL_EXPORT CModioString* GetModioEditModParamsDescription(const CModioEditModParams* ModioEditModParams);
MODIODLL_EXPORT CModioString* GetModioEditModParamsHomepageURL(const CModioEditModParams* ModioEditModParams);
MODIODLL_EXPORT COptionalModioMaturityProfile GetModioEditModParamsMaturityRating(const CModioEditModParams* ModioEditModParams);
MODIODLL_EXPORT CModioString* GetModioEditModParamsMetadataBlob(const CModioEditModParams* ModioEditModParams);
MODIODLL_EXPORT CModioString* GetModioEditModParamsLogoPath(const CModioEditModParams* ModioEditModParams);
MODIODLL_EXPORT void SetModioEditModParamsName(CModioEditModParams* Item, CModioString const* Name);
MODIODLL_EXPORT void SetModioEditModParamsSummary(CModioEditModParams* Item, CModioString const* Summary);
MODIODLL_EXPORT void SetModioEditModParamsNamePath(CModioEditModParams* Item, CModioString const* NamePath);
MODIODLL_EXPORT void SetModioEditModParamsDescription(CModioEditModParams* Item, CModioString const* Description);
MODIODLL_EXPORT void SetModioEditModParamsHomepageURL(CModioEditModParams* Item, CModioString const* HomepageURL);
MODIODLL_EXPORT void SetModioEditModParamsMaturityRating(CModioEditModParams* Item, CModioMaturityProfile const* MaturityRating);
MODIODLL_EXPORT void SetModioEditModParamsMetadataBlob(CModioEditModParams* Item, CModioString const* MetadataBlob);
MODIODLL_EXPORT void SetModioEditModParamsLogoPath(CModioEditModParams* Item, CModioString const* LogoPath);
MODIODLL_EXPORT void SetModioEditModParamsVisibility(CModioEditModParams* Item, EModioObjectVisibility const* Visibility);

MODIODLL_EXPORT CModioIcon* CreateModioIcon();
MODIODLL_EXPORT void ReleaseModioIcon(CModioIcon* Item);
MODIODLL_EXPORT CModioIcon* CopyModioIcon(const CModioIcon* Item);

MODIODLL_EXPORT CModioString* GetModioIconFilename(const CModioIcon* ModioIcon);
MODIODLL_EXPORT CModioString* GetModioIconOriginal(const CModioIcon* ModioIcon);
MODIODLL_EXPORT CModioString* GetModioIconThumb64(const CModioIcon* ModioIcon);
MODIODLL_EXPORT CModioString* GetModioIconThumb128(const CModioIcon* ModioIcon);
MODIODLL_EXPORT CModioString* GetModioIconThumb256(const CModioIcon* ModioIcon);
MODIODLL_EXPORT void SetModioIconFilename(CModioIcon* Item, CModioString const* Filename);
MODIODLL_EXPORT void SetModioIconOriginal(CModioIcon* Item, CModioString const* Original);
MODIODLL_EXPORT void SetModioIconThumb64(CModioIcon* Item, CModioString const* Thumb64);
MODIODLL_EXPORT void SetModioIconThumb128(CModioIcon* Item, CModioString const* Thumb128);
MODIODLL_EXPORT void SetModioIconThumb256(CModioIcon* Item, CModioString const* Thumb256);

MODIODLL_EXPORT CModioLogo* CreateModioLogo();
MODIODLL_EXPORT void ReleaseModioLogo(CModioLogo* Item);
MODIODLL_EXPORT CModioLogo* CopyModioLogo(const CModioLogo* Item);

MODIODLL_EXPORT CModioString* GetModioLogoFilename(const CModioLogo* ModioLogo);
MODIODLL_EXPORT CModioString* GetModioLogoOriginal(const CModioLogo* ModioLogo);
MODIODLL_EXPORT CModioString* GetModioLogoThumb320(const CModioLogo* ModioLogo);
MODIODLL_EXPORT CModioString* GetModioLogoThumb640(const CModioLogo* ModioLogo);
MODIODLL_EXPORT CModioString* GetModioLogoThumb1280(const CModioLogo* ModioLogo);
MODIODLL_EXPORT void SetModioLogoFilename(CModioLogo* Item, CModioString const* Filename);
MODIODLL_EXPORT void SetModioLogoOriginal(CModioLogo* Item, CModioString const* Original);
MODIODLL_EXPORT void SetModioLogoThumb320(CModioLogo* Item, CModioString const* Thumb320);
MODIODLL_EXPORT void SetModioLogoThumb640(CModioLogo* Item, CModioString const* Thumb640);
MODIODLL_EXPORT void SetModioLogoThumb1280(CModioLogo* Item, CModioString const* Thumb1280);

MODIODLL_EXPORT CModioHeaderImage* CreateModioHeaderImage();
MODIODLL_EXPORT void ReleaseModioHeaderImage(CModioHeaderImage* Item);
MODIODLL_EXPORT CModioHeaderImage* CopyModioHeaderImage(const CModioHeaderImage* Item);

MODIODLL_EXPORT CModioString* GetModioHeaderImageFilename(const CModioHeaderImage* HeaderImage);
MODIODLL_EXPORT CModioString* GetModioHeaderImageOriginal(const CModioHeaderImage* HeaderImage);
MODIODLL_EXPORT void SetModioHeaderImageFilename(CModioHeaderImage* Item, CModioString const* Filename);
MODIODLL_EXPORT void SetModioHeaderImageOriginal(CModioHeaderImage* Item, CModioString const* Original);

MODIODLL_EXPORT CModioOtherUrl* CreateModioOtherUrl();
MODIODLL_EXPORT void ReleaseModioOtherUrl(CModioOtherUrl* Item);
MODIODLL_EXPORT CModioOtherUrl* CopyModioOtherUrl(const CModioOtherUrl* Item);

MODIODLL_EXPORT CModioString* GetModioOtherUrlLabel(const CModioOtherUrl* OtherUrl);
MODIODLL_EXPORT CModioString* GetModioOtherUrlUrl(const CModioOtherUrl* OtherUrl);
MODIODLL_EXPORT void SetModioOtherUrlLabel(CModioOtherUrl* Item, CModioString const* Label);
MODIODLL_EXPORT void SetModioOtherUrlUrl(CModioOtherUrl* Item, CModioString const* Url);

MODIODLL_EXPORT CModioOtherUrlList* CreateModioOtherUrlList();
MODIODLL_EXPORT void ReleaseModioOtherUrlList(CModioOtherUrlList* Item);
MODIODLL_EXPORT CModioOtherUrlList* CopyModioOtherUrlList(const CModioOtherUrlList* Item);
MODIODLL_EXPORT CModioOtherUrl* GetModioOtherUrlListOtherURLbyIndex(const CModioOtherUrlList* ModioOtherUrlList, uint64_t Index);
MODIODLL_EXPORT void  SetModioOtherUrlListOtherURLbyIndex(CModioOtherUrlList* ModioOtherUrlList, uint64_t Index, CModioOtherUrl const* Value);
MODIODLL_EXPORT uint64_t GetModioOtherUrlListCount(const CModioOtherUrlList* ModioOtherUrlList);
MODIODLL_EXPORT void SetModioOtherUrlListCount(CModioOtherUrlList* ModioOtherUrlList, uint64_t Count);


MODIODLL_EXPORT CModioTheme* CreateModioTheme();
MODIODLL_EXPORT void ReleaseModioTheme(CModioTheme* Item);
MODIODLL_EXPORT CModioTheme* CopyModioTheme(const CModioTheme* Item);

MODIODLL_EXPORT CModioString* GetModioThemePrimary(const CModioTheme* Theme);
MODIODLL_EXPORT CModioString* GetModioThemeDark(const CModioTheme* Theme);
MODIODLL_EXPORT CModioString* GetModioThemeLight(const CModioTheme* Theme);
MODIODLL_EXPORT CModioString* GetModioThemeSuccess(const CModioTheme* Theme);
MODIODLL_EXPORT CModioString* GetModioThemeWarning(const CModioTheme* Theme);
MODIODLL_EXPORT CModioString* GetModioThemeDanger(const CModioTheme* Theme);
MODIODLL_EXPORT void SetModioThemePrimary(CModioTheme* Item, CModioString const* Primary);
MODIODLL_EXPORT void SetModioThemeDark(CModioTheme* Item, CModioString const* Dark);
MODIODLL_EXPORT void SetModioThemeLight(CModioTheme* Item, CModioString const* Light);
MODIODLL_EXPORT void SetModioThemeSuccess(CModioTheme* Item, CModioString const* Success);
MODIODLL_EXPORT void SetModioThemeWarning(CModioTheme* Item, CModioString const* Warning);
MODIODLL_EXPORT void SetModioThemeDanger(CModioTheme* Item, CModioString const* Danger);

MODIODLL_EXPORT CModioGameStats* CreateModioGameStats();
MODIODLL_EXPORT void ReleaseModioGameStats(CModioGameStats* Item);
MODIODLL_EXPORT CModioGameStats* CopyModioGameStats(const CModioGameStats* Item);

MODIODLL_EXPORT CModioGameID* GetModioGameStatsGameID(const CModioGameStats* GameStats);
MODIODLL_EXPORT int64_t GetModioGameStatsModCountTotal(const CModioGameStats* GameStats);
MODIODLL_EXPORT int64_t GetModioGameStatsModDownloadsToday(const CModioGameStats* GameStats);
MODIODLL_EXPORT int64_t GetModioGameStatsModDownloadsTotal(const CModioGameStats* GameStats);
MODIODLL_EXPORT int64_t GetModioGameStatsModDownloadsDailyAverage(const CModioGameStats* GameStats);
MODIODLL_EXPORT int64_t GetModioGameStatsModSubscribersTotal(const CModioGameStats* GameStats);
MODIODLL_EXPORT int64_t GetModioGameStatsDateExpires(const CModioGameStats* GameStats);
MODIODLL_EXPORT void SetModioGameStatsGameID(CModioGameStats* Item, CModioGameID const* GameID);
MODIODLL_EXPORT void SetModioGameStatsModCountTotal(CModioGameStats* Item, int64_t ModCountTotal);
MODIODLL_EXPORT void SetModioGameStatsModDownloadsToday(CModioGameStats* Item, int64_t ModDownloadsToday);
MODIODLL_EXPORT void SetModioGameStatsModDownloadsTotal(CModioGameStats* Item, int64_t ModDownloadsTotal);
MODIODLL_EXPORT void SetModioGameStatsModDownloadsDailyAverage(CModioGameStats* Item, int64_t ModDownloadsDailyAverage);
MODIODLL_EXPORT void SetModioGameStatsModSubscribersTotal(CModioGameStats* Item, int64_t ModSubscribersTotal);
MODIODLL_EXPORT void SetModioGameStatsDateExpires(CModioGameStats* Item, int64_t DateExpires);

MODIODLL_EXPORT CModioTermsOfUse* CreateModioTermsOfUse();
MODIODLL_EXPORT void ReleaseModioTermsOfUse(CModioTermsOfUse* Item);
MODIODLL_EXPORT CModioTermsOfUse* CopyModioTermsOfUse(const CModioTermsOfUse* Item);

MODIODLL_EXPORT CModioLink* GetModioTermsOfUseWebsiteLink(const CModioTermsOfUse* ModioTermsOfUse);
MODIODLL_EXPORT CModioLink* GetModioTermsOfUseTermsLink(const CModioTermsOfUse* ModioTermsOfUse);
MODIODLL_EXPORT CModioLink* GetModioTermsOfUsePrivacyLink(const CModioTermsOfUse* ModioTermsOfUse);
MODIODLL_EXPORT CModioLink* GetModioTermsOfUseManageLink(const CModioTermsOfUse* ModioTermsOfUse);
MODIODLL_EXPORT void SetModioTermsOfUseWebsiteLink(CModioTermsOfUse* Item, CModioLink const* WebsiteLink);
MODIODLL_EXPORT void SetModioTermsOfUseTermsLink(CModioTermsOfUse* Item, CModioLink const* TermsLink);
MODIODLL_EXPORT void SetModioTermsOfUsePrivacyLink(CModioTermsOfUse* Item, CModioLink const* PrivacyLink);
MODIODLL_EXPORT void SetModioTermsOfUseManageLink(CModioTermsOfUse* Item, CModioLink const* ManageLink);

MODIODLL_EXPORT CModioModTagList* CreateModioModTagList();
MODIODLL_EXPORT void ReleaseModioModTagList(CModioModTagList* Item);
MODIODLL_EXPORT CModioModTagList* CopyModioModTagList(const CModioModTagList* Item);
MODIODLL_EXPORT CModioTag* GetModioModTagListTagByIndex(const CModioModTagList* ModioModTagList, uint64_t Index);
MODIODLL_EXPORT void  SetModioModTagListTagByIndex(CModioModTagList* ModioModTagList, uint64_t Index, CModioTag const* Value);
MODIODLL_EXPORT uint64_t GetModioModTagListCount(const CModioModTagList* ModioModTagList);
MODIODLL_EXPORT void SetModioModTagListCount(CModioModTagList* ModioModTagList, uint64_t Count);


MODIODLL_EXPORT CModioModStats* CreateModioModStats();
MODIODLL_EXPORT void ReleaseModioModStats(CModioModStats* Item);
MODIODLL_EXPORT CModioModStats* CopyModioModStats(const CModioModStats* Item);

MODIODLL_EXPORT int64_t GetModioModStatsPopularityRankPosition(const CModioModStats* ModioModStats);
MODIODLL_EXPORT int64_t GetModioModStatsPopularityRankTotalMods(const CModioModStats* ModioModStats);
MODIODLL_EXPORT int64_t GetModioModStatsDownloadsTotal(const CModioModStats* ModioModStats);
MODIODLL_EXPORT int64_t GetModioModStatsSubscribersTotal(const CModioModStats* ModioModStats);
MODIODLL_EXPORT int64_t GetModioModStatsRatingTotal(const CModioModStats* ModioModStats);
MODIODLL_EXPORT int64_t GetModioModStatsRatingPositive(const CModioModStats* ModioModStats);
MODIODLL_EXPORT int64_t GetModioModStatsRatingNegative(const CModioModStats* ModioModStats);
MODIODLL_EXPORT int64_t GetModioModStatsRatingPercentagePositive(const CModioModStats* ModioModStats);
MODIODLL_EXPORT double GetModioModStatsRatingWeightedAggregate(const CModioModStats* ModioModStats);
MODIODLL_EXPORT CModioString* GetModioModStatsRatingDisplayText(const CModioModStats* ModioModStats);
MODIODLL_EXPORT void SetModioModStatsPopularityRankPosition(CModioModStats* Item, int64_t PopularityRankPosition);
MODIODLL_EXPORT void SetModioModStatsPopularityRankTotalMods(CModioModStats* Item, int64_t PopularityRankTotalMods);
MODIODLL_EXPORT void SetModioModStatsDownloadsTotal(CModioModStats* Item, int64_t DownloadsTotal);
MODIODLL_EXPORT void SetModioModStatsSubscribersTotal(CModioModStats* Item, int64_t SubscribersTotal);
MODIODLL_EXPORT void SetModioModStatsRatingTotal(CModioModStats* Item, int64_t RatingTotal);
MODIODLL_EXPORT void SetModioModStatsRatingPositive(CModioModStats* Item, int64_t RatingPositive);
MODIODLL_EXPORT void SetModioModStatsRatingNegative(CModioModStats* Item, int64_t RatingNegative);
MODIODLL_EXPORT void SetModioModStatsRatingPercentagePositive(CModioModStats* Item, int64_t RatingPercentagePositive);
MODIODLL_EXPORT void SetModioModStatsRatingWeightedAggregate(CModioModStats* Item, double RatingWeightedAggregate);
MODIODLL_EXPORT void SetModioModStatsRatingDisplayText(CModioModStats* Item, CModioString const* RatingDisplayText);

MODIODLL_EXPORT CModioGameInfo* CreateModioGameInfo();
MODIODLL_EXPORT void ReleaseModioGameInfo(CModioGameInfo* Item);
MODIODLL_EXPORT CModioGameInfo* CopyModioGameInfo(const CModioGameInfo* Item);

MODIODLL_EXPORT CModioGameID* GetModioGameInfoGameID(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT int64_t GetModioGameInfoDateAdded(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT int64_t GetModioGameInfoDateUpdated(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT int64_t GetModioGameInfoDateLive(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioString* GetModioGameInfoUgcName(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioIcon* GetModioGameInfoIcon(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioLogo* GetModioGameInfoLogo(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioHeaderImage* GetModioGameInfoHeaderImage(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioString* GetModioGameInfoName(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioString* GetModioGameInfoSummary(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioString* GetModioGameInfoInstructions(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioString* GetModioGameInfoInstructionsUrl(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioString* GetModioGameInfoProfileUrl(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioTheme* GetModioGameInfoTheme(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioGameStats* GetModioGameInfoStats(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioOtherUrlList* GetModioGameInfoOtherUrls(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioGamePlatformList* GetModioGameInfoPlatformSupport(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioGameCommunityOptionsFlags GetModioGameInfoCommunityOptions(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioMaturityProfile GetModioGameInfoMaturityOptions(const CModioGameInfo* ModInfo);
MODIODLL_EXPORT CModioString* GetModioGameInfoVirtualTokenName(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT CModioModTagInfoList* GetModioGameInfoTagOptions(const CModioGameInfo* ModioGameInfo);
MODIODLL_EXPORT void SetModioGameInfoGameID(CModioGameInfo* Item, CModioGameID const* GameID);
MODIODLL_EXPORT void SetModioGameInfoDateAdded(CModioGameInfo* Item, int64_t DateAdded);
MODIODLL_EXPORT void SetModioGameInfoDateUpdated(CModioGameInfo* Item, int64_t DateUpdated);
MODIODLL_EXPORT void SetModioGameInfoDateLive(CModioGameInfo* Item, int64_t DateLive);
MODIODLL_EXPORT void SetModioGameInfoUgcName(CModioGameInfo* Item, CModioString const* UgcName);
MODIODLL_EXPORT void SetModioGameInfoIcon(CModioGameInfo* Item, CModioIcon const* Icon);
MODIODLL_EXPORT void SetModioGameInfoLogo(CModioGameInfo* Item, CModioLogo const* Logo);
MODIODLL_EXPORT void SetModioGameInfoHeaderImage(CModioGameInfo* Item, CModioHeaderImage const* HeaderImage);
MODIODLL_EXPORT void SetModioGameInfoName(CModioGameInfo* Item, CModioString const* Name);
MODIODLL_EXPORT void SetModioGameInfoSummary(CModioGameInfo* Item, CModioString const* Summary);
MODIODLL_EXPORT void SetModioGameInfoInstructions(CModioGameInfo* Item, CModioString const* Instructions);
MODIODLL_EXPORT void SetModioGameInfoInstructionsUrl(CModioGameInfo* Item, CModioString const* InstructionsUrl);
MODIODLL_EXPORT void SetModioGameInfoProfileUrl(CModioGameInfo* Item, CModioString const* ProfileUrl);
MODIODLL_EXPORT void SetModioGameInfoTheme(CModioGameInfo* Item, CModioTheme const* Theme);
MODIODLL_EXPORT void SetModioGameInfoStats(CModioGameInfo* Item, CModioGameStats const* Stats);
MODIODLL_EXPORT void SetModioGameInfoOtherUrls(CModioGameInfo* Item, CModioOtherUrlList const* OtherUrls);
MODIODLL_EXPORT void SetModioGameInfoPlatformSupport(CModioGameInfo* Item, CModioGamePlatformList const* PlatformSupport);
MODIODLL_EXPORT void SetModioGameInfoCommunityOptions(CModioGameInfo* Item, CModioGameCommunityOptionsFlags CommunityOptions);
MODIODLL_EXPORT void SetModioGameInfoMaturityOptions(CModioGameInfo* Item, CModioMaturityProfile MaturityOptions);
MODIODLL_EXPORT void SetModioGameInfoVirtualTokenName(CModioGameInfo* Item, CModioString const* VirtualTokenName);
MODIODLL_EXPORT void SetModioGameInfoTagOptions(CModioGameInfo* Item, CModioModTagInfoList const* TagOptions);

MODIODLL_EXPORT CModioModLogo* CreateModioModLogo();
MODIODLL_EXPORT void ReleaseModioModLogo(CModioModLogo* Item);
MODIODLL_EXPORT CModioModLogo* CopyModioModLogo(const CModioModLogo* Item);

MODIODLL_EXPORT CModioString* GetModioModLogoFilename(const CModioModLogo* ModioModLogo);
MODIODLL_EXPORT CModioString* GetModioModLogoOriginal(const CModioModLogo* ModioModLogo);
MODIODLL_EXPORT CModioString* GetModioModLogoThumb320x180(const CModioModLogo* ModioModLogo);
MODIODLL_EXPORT CModioString* GetModioModLogoThumb640x360(const CModioModLogo* ModioModLogo);
MODIODLL_EXPORT CModioString* GetModioModLogoThumb1280x720(const CModioModLogo* ModioModLogo);
MODIODLL_EXPORT void SetModioModLogoFilename(CModioModLogo* Item, CModioString const* Filename);
MODIODLL_EXPORT void SetModioModLogoOriginal(CModioModLogo* Item, CModioString const* Original);
MODIODLL_EXPORT void SetModioModLogoThumb320x180(CModioModLogo* Item, CModioString const* Thumb320x180);
MODIODLL_EXPORT void SetModioModLogoThumb640x360(CModioModLogo* Item, CModioString const* Thumb640x360);
MODIODLL_EXPORT void SetModioModLogoThumb1280x720(CModioModLogo* Item, CModioString const* Thumb1280x720);

MODIODLL_EXPORT CModioKVP* CreateModioKVP();
MODIODLL_EXPORT void ReleaseModioKVP(CModioKVP* Item);
MODIODLL_EXPORT CModioKVP* CopyModioKVP(const CModioKVP* Item);

MODIODLL_EXPORT CModioString* GetModioKVPKey(const CModioKVP* ModioKVP);
MODIODLL_EXPORT CModioString* GetModioKVPValue(const CModioKVP* ModioKVP);
MODIODLL_EXPORT void SetModioKVPKey(CModioKVP* Item, CModioString const* Key);
MODIODLL_EXPORT void SetModioKVPValue(CModioKVP* Item, CModioString const* Value);

MODIODLL_EXPORT CModioMetadataKVP* CreateModioMetadataKVP();
MODIODLL_EXPORT void ReleaseModioMetadataKVP(CModioMetadataKVP* Item);
MODIODLL_EXPORT CModioMetadataKVP* CopyModioMetadataKVP(const CModioMetadataKVP* Item);
MODIODLL_EXPORT CModioKVP* GetModioMetadataKVPKVPByIndex(const CModioMetadataKVP* ModioMetadataKVP, uint64_t Index);
MODIODLL_EXPORT void  SetModioMetadataKVPKVPByIndex(CModioMetadataKVP* ModioMetadataKVP, uint64_t Index, CModioKVP const* Value);
MODIODLL_EXPORT uint64_t GetModioMetadataKVPCount(const CModioMetadataKVP* ModioMetadataKVP);
MODIODLL_EXPORT void SetModioMetadataKVPCount(CModioMetadataKVP* ModioMetadataKVP, uint64_t Count);


MODIODLL_EXPORT CModioFileInfo* CreateModioFileInfo();
MODIODLL_EXPORT void ReleaseModioFileInfo(CModioFileInfo* Item);
MODIODLL_EXPORT CModioFileInfo* CopyModioFileInfo(const CModioFileInfo* Item);

MODIODLL_EXPORT CModioFileMetadataID* GetModioFileInfoMetadataID(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT CModioModID* GetModioFileInfoModID(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT int64_t GetModioFileInfoDateAdded(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT EModioVirusScanStatus GetModioFileInfoVirusScanStatus(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT EModioVirusStatus GetModioFileInfoVirusStatus(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT int64_t GetModioFileInfoFileSize(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT int64_t GetModioFileInfoFileSizeUncompressed(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT CModioString* GetModioFileInfoFilename(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT CModioString* GetModioFileInfoVersion(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT CModioString* GetModioFileInfoChangelog(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT CModioString* GetModioFileInfoMetadataBlob(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT CModioString* GetModioFileInfoDownloadBinaryURL(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT int64_t GetModioFileInfoDownloadExpiryDate(const CModioFileInfo* ModioFileInfo);
MODIODLL_EXPORT void SetModioFileInfoMetadataID(CModioFileInfo* Item, CModioFileMetadataID const* MetadataID);
MODIODLL_EXPORT void SetModioFileInfoModID(CModioFileInfo* Item, CModioModID const* ModID);
MODIODLL_EXPORT void SetModioFileInfoDateAdded(CModioFileInfo* Item, int64_t DateAdded);
MODIODLL_EXPORT void SetModioFileInfoVirusScanStatus(CModioFileInfo* Item, EModioVirusScanStatus VirusScanStatus);
MODIODLL_EXPORT void SetModioFileInfoVirusStatus(CModioFileInfo* Item, EModioVirusStatus VirusStatus);
MODIODLL_EXPORT void SetModioFileInfoFileSize(CModioFileInfo* Item, int64_t FileSize);
MODIODLL_EXPORT void SetModioFileInfoFileSizeUncompressed(CModioFileInfo* Item, int64_t FileSizeUncompressed);
MODIODLL_EXPORT void SetModioFileInfoFilename(CModioFileInfo* Item, CModioString const* Filename);
MODIODLL_EXPORT void SetModioFileInfoVersion(CModioFileInfo* Item, CModioString const* Version);
MODIODLL_EXPORT void SetModioFileInfoChangelog(CModioFileInfo* Item, CModioString const* Changelog);
MODIODLL_EXPORT void SetModioFileInfoMetadataBlob(CModioFileInfo* Item, CModioString const* MetadataBlob);
MODIODLL_EXPORT void SetModioFileInfoDownloadBinaryURL(CModioFileInfo* Item, CModioString const* DownloadBinaryURL);
MODIODLL_EXPORT void SetModioFileInfoDownloadExpiryDate(CModioFileInfo* Item, int64_t DownloadExpiryDate);

MODIODLL_EXPORT CModioUser* CreateModioUser();
MODIODLL_EXPORT void ReleaseModioUser(CModioUser* Item);
MODIODLL_EXPORT CModioUser* CopyModioUser(const CModioUser* Item);

MODIODLL_EXPORT CModioUserID* GetModioUserUserID(const CModioUser* ModioUser);
MODIODLL_EXPORT CModioString* GetModioUserUsername(const CModioUser* ModioUser);
MODIODLL_EXPORT CModioAuthToken* GetModioUserAuthToken(const CModioUser* ModioUser);
MODIODLL_EXPORT int64_t GetModioUserDateOnline(const CModioUser* ModioUser);
MODIODLL_EXPORT CModioString* GetModioUserProfileURL(const CModioUser* ModioUser);
MODIODLL_EXPORT CModioAvatar* GetModioUserAvatar(const CModioUser* ModioUser);
MODIODLL_EXPORT CModioString* GetModioUserDisplayNamePortal(const CModioUser* ModioUser);
MODIODLL_EXPORT void SetModioUserUserID(CModioUser* Item, CModioUserID const* UserID);
MODIODLL_EXPORT void SetModioUserUsername(CModioUser* Item, CModioString const* Username);
MODIODLL_EXPORT void SetModioUserAuthToken(CModioUser* Item, CModioAuthToken const* AuthToken);
MODIODLL_EXPORT void SetModioUserDateOnline(CModioUser* Item, int64_t DateOnline);
MODIODLL_EXPORT void SetModioUserProfileURL(CModioUser* Item, CModioString const* ProfileURL);
MODIODLL_EXPORT void SetModioUserAvatar(CModioUser* Item, CModioAvatar const* Avatar);
MODIODLL_EXPORT void SetModioUserDisplayNamePortal(CModioUser* Item, CModioString const* DisplayNamePortal);

MODIODLL_EXPORT CModioFilterParams* CreateModioFilterParams();
MODIODLL_EXPORT void ReleaseModioFilterParams(CModioFilterParams* Item);
MODIODLL_EXPORT CModioFilterParams* CopyModioFilterParams(const CModioFilterParams* Item);
MODIODLL_EXPORT void ModioFilterParamsSortBy(CModioFilterParams *Item, EModioSortFieldType ByField, EModioSortDirection ByDirection);
MODIODLL_EXPORT void ModioFilterParamsNameContains(CModioFilterParams *Item, CModioStringList const* SearchString);
MODIODLL_EXPORT void ModioFilterParamsMatchingAuthor(CModioFilterParams *Item, CModioUserID const* UserId);
MODIODLL_EXPORT void ModioFilterParamsMatchingAuthors(CModioFilterParams *Item, CModioUserIDList const* UserIds);
MODIODLL_EXPORT void ModioFilterParamsMatchingIDs(CModioFilterParams *Item, CModioModIDList const* IDSet);
MODIODLL_EXPORT void ModioFilterParamsExcludingIDs(CModioFilterParams *Item, CModioModIDList const* IDSet);
MODIODLL_EXPORT void ModioFilterParamsWithTags(CModioFilterParams *Item, CModioStringList const* UserId);
MODIODLL_EXPORT void ModioFilterParamsWithoutTags(CModioFilterParams *Item, CModioStringList const* UserId);
MODIODLL_EXPORT void ModioFilterParamsMetadataLike(CModioFilterParams *Item, CModioString const* SearchString);
MODIODLL_EXPORT void ModioFilterParamsIndexedResults(CModioFilterParams *Item, size_t StartIndex, size_t ResultCount);
MODIODLL_EXPORT void ModioFilterParamsPagedResults(CModioFilterParams *Item, size_t PageNumber, size_t PageSize);
MODIODLL_EXPORT void ModioFilterParamsRevenueType(CModioFilterParams *Item, EModioRevenueFilterType ByRevenue);
MODIODLL_EXPORT void ModioFilterParamsDisallowMatureContent(CModioFilterParams *Item);


MODIODLL_EXPORT CModioAvatar* CreateModioAvatar();
MODIODLL_EXPORT void ReleaseModioAvatar(CModioAvatar* Item);
MODIODLL_EXPORT CModioAvatar* CopyModioAvatar(const CModioAvatar* Item);

MODIODLL_EXPORT CModioString* GetModioAvatarFilename(const CModioAvatar* ModioAvatar);
MODIODLL_EXPORT CModioString* GetModioAvatarFullSizeURL(const CModioAvatar* ModioAvatar);
MODIODLL_EXPORT CModioString* GetModioAvatarURL50x50(const CModioAvatar* ModioAvatar);
MODIODLL_EXPORT CModioString* GetModioAvatarURL100x100(const CModioAvatar* ModioAvatar);
MODIODLL_EXPORT void SetModioAvatarFilename(CModioAvatar* Item, CModioString const* Filename);
MODIODLL_EXPORT void SetModioAvatarFullSizeURL(CModioAvatar* Item, CModioString const* FullSizeURL);
MODIODLL_EXPORT void SetModioAvatarURL50x50(CModioAvatar* Item, CModioString const* URL50x50);
MODIODLL_EXPORT void SetModioAvatarURL100x100(CModioAvatar* Item, CModioString const* URL100x100);

MODIODLL_EXPORT CModioFieldError* CreateModioFieldError();
MODIODLL_EXPORT void ReleaseModioFieldError(CModioFieldError* Item);
MODIODLL_EXPORT CModioFieldError* CopyModioFieldError(const CModioFieldError* Item);


MODIODLL_EXPORT CModioValidationError* CreateModioValidationError();
MODIODLL_EXPORT void ReleaseModioValidationError(CModioValidationError* Item);
MODIODLL_EXPORT CModioValidationError* CopyModioValidationError(const CModioValidationError* Item);

MODIODLL_EXPORT CModioString* GetModioValidationErrorFieldName(const CModioValidationError* ValidationError);
MODIODLL_EXPORT CModioString* GetModioValidationErrorError(const CModioValidationError* ValidationError);
MODIODLL_EXPORT void SetModioValidationErrorFieldName(CModioValidationError* Item, CModioString const* FieldName);
MODIODLL_EXPORT void SetModioValidationErrorError(CModioValidationError* Item, CModioString const* Error);

MODIODLL_EXPORT CModioValidationErrorList* CreateModioValidationErrorList();
MODIODLL_EXPORT void ReleaseModioValidationErrorList(CModioValidationErrorList* Item);
MODIODLL_EXPORT CModioValidationErrorList* CopyModioValidationErrorList(const CModioValidationErrorList* Item);
MODIODLL_EXPORT CModioValidationError* GetModioValidationErrorListErrorByIndex(const CModioValidationErrorList* ValidationErrorList, uint64_t Index);
MODIODLL_EXPORT void  SetModioValidationErrorListErrorByIndex(CModioValidationErrorList* ValidationErrorList, uint64_t Index, CModioValidationError const* Value);
MODIODLL_EXPORT uint64_t GetModioValidationErrorListCount(const CModioValidationErrorList* ValidationErrorList);
MODIODLL_EXPORT void SetModioValidationErrorListCount(CModioValidationErrorList* ValidationErrorList, uint64_t Count);


MODIODLL_EXPORT CModioModProgressInfo* CreateModioModProgressInfo(const CModioModID* ID);
MODIODLL_EXPORT void ReleaseModioModProgressInfo(CModioModProgressInfo* Item);
MODIODLL_EXPORT CModioModProgressInfo* CopyModioModProgressInfo(const CModioModProgressInfo* Item);

MODIODLL_EXPORT CModioModID* GetModioModProgressInfoID(const CModioModProgressInfo* ProgressInfo);
MODIODLL_EXPORT void SetModioModProgressInfoID(CModioModProgressInfo* Item, CModioModID const* ID);

MODIODLL_EXPORT CModioStringList* CreateModioStringList();
MODIODLL_EXPORT void ReleaseModioStringList(CModioStringList* Item);
MODIODLL_EXPORT CModioStringList* CopyModioStringList(const CModioStringList* Item);
MODIODLL_EXPORT CModioString* GetModioStringListStringByIndex(const CModioStringList* ModioStringList, uint64_t Index);
MODIODLL_EXPORT void  SetModioStringListStringByIndex(CModioStringList* ModioStringList, uint64_t Index, CModioString const* Value);
MODIODLL_EXPORT uint64_t GetModioStringListCount(const CModioStringList* ModioStringList);
MODIODLL_EXPORT void SetModioStringListCount(CModioStringList* ModioStringList, uint64_t Count);


MODIODLL_EXPORT CModioStringMap* CreateModioStringMap();
MODIODLL_EXPORT void ReleaseModioStringMap(CModioStringMap* Item);
MODIODLL_EXPORT CModioStringMap* CopyModioStringMap(const CModioStringMap* Item);
MODIODLL_EXPORT void ModioStringMapSetKeyValue(CModioStringMap* Item, const char* Key, const char* Value);
MODIODLL_EXPORT const char* ModioStringMapGetKeyValue(CModioStringMap* Item, const char* Key);
MODIODLL_EXPORT CModioStringMapIterator* ModioStringMapBegin(CModioStringMap* Item);
MODIODLL_EXPORT bool ModioStringMapNext(CModioStringMapIterator* Iterator);
MODIODLL_EXPORT const char* ModioStringMapGetKey(CModioStringMapIterator* Iterator);
MODIODLL_EXPORT const char* ModioStringMapGetValue(CModioStringMapIterator* Iterator);
MODIODLL_EXPORT void ReleaseModioStringMapIterator(CModioStringMapIterator* Item);


MODIODLL_EXPORT CModioAuthenticationParams* CreateModioAuthenticationParams(const char* OAuthTokenData, size_t OAuthTokenLength);
MODIODLL_EXPORT void ReleaseModioAuthenticationParams(CModioAuthenticationParams* Item);
MODIODLL_EXPORT CModioAuthenticationParams* CopyModioAuthenticationParams(const CModioAuthenticationParams* Item);

MODIODLL_EXPORT CModioString* GetModioAuthenticationParamsUserEmail(const CModioAuthenticationParams* AuthenticationParams);
MODIODLL_EXPORT bool GetModioAuthenticationParamsUserHasAcceptedTerms(const CModioAuthenticationParams* AuthenticationParams);
MODIODLL_EXPORT bool GetModioAuthenticationParamsURLEncodeAuthToken(const CModioAuthenticationParams* AuthenticationParams);
MODIODLL_EXPORT CModioStringMap* GetModioAuthenticationParamsExtendedParameters(const CModioAuthenticationParams* AuthenticationParams);
MODIODLL_EXPORT void SetModioAuthenticationParamsUserEmail(CModioAuthenticationParams* Item, CModioString const* UserEmail);
MODIODLL_EXPORT void SetModioAuthenticationParamsUserHasAcceptedTerms(CModioAuthenticationParams* Item, bool UserHasAcceptedTerms);
MODIODLL_EXPORT void SetModioAuthenticationParamsURLEncodeAuthToken(CModioAuthenticationParams* Item, bool URLEncodeAuthToken);
MODIODLL_EXPORT void SetModioAuthenticationParamsExtendedParameters(CModioAuthenticationParams* Item, CModioStringMap const* ExtendedParameters);

MODIODLL_EXPORT CModioModInfoList* CreateModioModInfoList();
MODIODLL_EXPORT void ReleaseModioModInfoList(CModioModInfoList* Item);
MODIODLL_EXPORT CModioModInfoList* CopyModioModInfoList(const CModioModInfoList* Item);
MODIODLL_EXPORT CModioModInfo* GetModioModInfoListModInfoByIndex(const CModioModInfoList* ModioModInfoList, uint64_t Index);
MODIODLL_EXPORT void  SetModioModInfoListModInfoByIndex(CModioModInfoList* ModioModInfoList, uint64_t Index, CModioModInfo const* Value);
MODIODLL_EXPORT uint64_t GetModioModInfoListCount(const CModioModInfoList* ModioModInfoList);
MODIODLL_EXPORT void SetModioModInfoListCount(CModioModInfoList* ModioModInfoList, uint64_t Count);


MODIODLL_EXPORT CModioUserList* CreateModioUserList();
MODIODLL_EXPORT void ReleaseModioUserList(CModioUserList* Item);
MODIODLL_EXPORT CModioUserList* CopyModioUserList(const CModioUserList* Item);
MODIODLL_EXPORT CModioUser* GetModioUserListUserByIndex(const CModioUserList* ModioUserList, uint64_t Index);
MODIODLL_EXPORT void  SetModioUserListUserByIndex(CModioUserList* ModioUserList, uint64_t Index, CModioUser const* Value);
MODIODLL_EXPORT uint64_t GetModioUserListCount(const CModioUserList* ModioUserList);
MODIODLL_EXPORT void SetModioUserListCount(CModioUserList* ModioUserList, uint64_t Count);


MODIODLL_EXPORT CModioGamePlatform* CreateModioGamePlatform();
MODIODLL_EXPORT void ReleaseModioGamePlatform(CModioGamePlatform* Item);
MODIODLL_EXPORT CModioGamePlatform* CopyModioGamePlatform(const CModioGamePlatform* Item);

MODIODLL_EXPORT EModioModfilePlatform GetModioGamePlatformPlatform(const CModioGamePlatform* GamePlatform);
MODIODLL_EXPORT bool GetModioGamePlatformLocked(const CModioGamePlatform* GamePlatform);
MODIODLL_EXPORT bool GetModioGamePlatformModerated(const CModioGamePlatform* GamePlatform);
MODIODLL_EXPORT void SetModioGamePlatformPlatform(CModioGamePlatform* Item, EModioModfilePlatform Platform);
MODIODLL_EXPORT void SetModioGamePlatformLocked(CModioGamePlatform* Item, bool Locked);
MODIODLL_EXPORT void SetModioGamePlatformModerated(CModioGamePlatform* Item, bool Moderated);

MODIODLL_EXPORT CModioModTagInfo* CreateModioModTagInfo();
MODIODLL_EXPORT void ReleaseModioModTagInfo(CModioModTagInfo* Item);
MODIODLL_EXPORT CModioModTagInfo* CopyModioModTagInfo(const CModioModTagInfo* Item);

MODIODLL_EXPORT CModioString* GetModioModTagInfoTagGroupName(const CModioModTagInfo* ModioModTagInfo);
MODIODLL_EXPORT CModioStringList* GetModioModTagInfoTagGroupValues(const CModioModTagInfo* ModioModTagInfo);
MODIODLL_EXPORT bool GetModioModTagInfoAllowMultipleSelection(const CModioModTagInfo* ModioModTagInfo);
MODIODLL_EXPORT void SetModioModTagInfoTagGroupName(CModioModTagInfo* Item, CModioString const* TagGroupName);
MODIODLL_EXPORT void SetModioModTagInfoTagGroupValues(CModioModTagInfo* Item, CModioStringList const* TagGroupValues);
MODIODLL_EXPORT void SetModioModTagInfoAllowMultipleSelection(CModioModTagInfo* Item, bool AllowMultipleSelection);

MODIODLL_EXPORT CModioModTagOptions* CreateModioModTagOptions();
MODIODLL_EXPORT void ReleaseModioModTagOptions(CModioModTagOptions* Item);
MODIODLL_EXPORT CModioModTagOptions* CopyModioModTagOptions(const CModioModTagOptions* Item);
MODIODLL_EXPORT CModioModTagInfo* GetModioModTagOptionsModTagInfoByIndex(const CModioModTagOptions* ModioModTagOptions, uint64_t Index);
MODIODLL_EXPORT void  SetModioModTagOptionsModTagInfoByIndex(CModioModTagOptions* ModioModTagOptions, uint64_t Index, CModioModTagInfo const* Value);
MODIODLL_EXPORT uint64_t GetModioModTagOptionsCount(const CModioModTagOptions* ModioModTagOptions);
MODIODLL_EXPORT void SetModioModTagOptionsCount(CModioModTagOptions* ModioModTagOptions, uint64_t Count);


MODIODLL_EXPORT CModioModTagInfoList* CreateModioModTagInfoList();
MODIODLL_EXPORT void ReleaseModioModTagInfoList(CModioModTagInfoList* Item);
MODIODLL_EXPORT CModioModTagInfoList* CopyModioModTagInfoList(const CModioModTagInfoList* Item);
MODIODLL_EXPORT CModioModTagInfo* GetModioModTagInfoListModTagInfoByIndex(const CModioModTagInfoList* ModioModTagInfoList, uint64_t Index);
MODIODLL_EXPORT void  SetModioModTagInfoListModTagInfoByIndex(CModioModTagInfoList* ModioModTagInfoList, uint64_t Index, CModioModTagInfo const* Value);
MODIODLL_EXPORT uint64_t GetModioModTagInfoListCount(const CModioModTagInfoList* ModioModTagInfoList);
MODIODLL_EXPORT void SetModioModTagInfoListCount(CModioModTagInfoList* ModioModTagInfoList, uint64_t Count);


MODIODLL_EXPORT CModioModCollectionMap* CreateModioModCollectionMap();
MODIODLL_EXPORT void ReleaseModioModCollectionMap(CModioModCollectionMap* Item);
MODIODLL_EXPORT CModioModCollectionMap* CopyModioModCollectionMap(const CModioModCollectionMap* Item);
MODIODLL_EXPORT void ModioModCollectionMapSetKeyValue(CModioModCollectionMap* Item, CModioModID const* Key, CModioModCollectionEntry const* Value);
MODIODLL_EXPORT CModioModCollectionEntry* ModioModCollectionMapGetKeyValue(CModioModCollectionMap* Item, CModioModID const* Key);
MODIODLL_EXPORT CModioModCollectionMapIterator* ModioModCollectionMapBegin(CModioModCollectionMap* Item);
MODIODLL_EXPORT bool ModioModCollectionMapNext(CModioModCollectionMapIterator* Iterator);
MODIODLL_EXPORT CModioModID* ModioModCollectionMapGetKey(CModioModCollectionMapIterator* Iterator);
MODIODLL_EXPORT CModioModCollectionEntry* ModioModCollectionMapGetValue(CModioModCollectionMapIterator* Iterator);
MODIODLL_EXPORT void ReleaseModioModCollectionMapIterator(CModioModCollectionMapIterator* Item);


MODIODLL_EXPORT CModioModDependency* CreateModioModDependency();
MODIODLL_EXPORT void ReleaseModioModDependency(CModioModDependency* Item);
MODIODLL_EXPORT CModioModDependency* CopyModioModDependency(const CModioModDependency* Item);

MODIODLL_EXPORT CModioModID* GetModioModDependencyModID(const CModioModDependency* ModioModDependency);
MODIODLL_EXPORT CModioString* GetModioModDependencyModName(const CModioModDependency* ModioModDependency);
MODIODLL_EXPORT int64_t GetModioModDependencyDateAdded(const CModioModDependency* ModioModDependency);
MODIODLL_EXPORT int64_t GetModioModDependencyDateUpdated(const CModioModDependency* ModioModDependency);
MODIODLL_EXPORT uint8_t GetModioModDependencyDependencyDepth(const CModioModDependency* ModioModDependency);
MODIODLL_EXPORT CModioLogo* GetModioModDependencyLogo(const CModioModDependency* ModioModDependency);
MODIODLL_EXPORT CModioFileInfo* GetModioModDependencyFileInfo(const CModioModDependency* ModioModDependency);
MODIODLL_EXPORT EModioModServerSideStatus GetModioModDependencyStatus(const CModioModDependency* ModioModDependency);
MODIODLL_EXPORT EModioObjectVisibility GetModioModDependencyVisibility(const CModioModDependency* ModioModDependency);
MODIODLL_EXPORT void SetModioModDependencyModID(CModioModDependency* Item, CModioModID const* ModID);
MODIODLL_EXPORT void SetModioModDependencyModName(CModioModDependency* Item, CModioString const* ModName);
MODIODLL_EXPORT void SetModioModDependencyDateAdded(CModioModDependency* Item, int64_t DateAdded);
MODIODLL_EXPORT void SetModioModDependencyDateUpdated(CModioModDependency* Item, int64_t DateUpdated);
MODIODLL_EXPORT void SetModioModDependencyDependencyDepth(CModioModDependency* Item, uint8_t DependencyDepth);
MODIODLL_EXPORT void SetModioModDependencyLogo(CModioModDependency* Item, CModioLogo const* Logo);
MODIODLL_EXPORT void SetModioModDependencyFileInfo(CModioModDependency* Item, CModioFileInfo const* FileInfo);
MODIODLL_EXPORT void SetModioModDependencyStatus(CModioModDependency* Item, EModioModServerSideStatus Status);
MODIODLL_EXPORT void SetModioModDependencyVisibility(CModioModDependency* Item, EModioObjectVisibility Visibility);

MODIODLL_EXPORT CModioModDependencyList* CreateModioModDependencyList();
MODIODLL_EXPORT void ReleaseModioModDependencyList(CModioModDependencyList* Item);
MODIODLL_EXPORT CModioModDependencyList* CopyModioModDependencyList(const CModioModDependencyList* Item);
MODIODLL_EXPORT CModioModDependency* GetModioModDependencyListDependencyByIndex(const CModioModDependencyList* ModDependencyList, uint64_t Index);
MODIODLL_EXPORT void  SetModioModDependencyListDependencyByIndex(CModioModDependencyList* ModDependencyList, uint64_t Index, CModioModDependency const* Value);
MODIODLL_EXPORT uint64_t GetModioModDependencyListCount(const CModioModDependencyList* ModDependencyList);
MODIODLL_EXPORT void SetModioModDependencyListCount(CModioModDependencyList* ModDependencyList, uint64_t Count);


MODIODLL_EXPORT CModioModCollectionEntry* CreateModioModCollectionEntry();
MODIODLL_EXPORT void ReleaseModioModCollectionEntry(CModioModCollectionEntry* Item);
MODIODLL_EXPORT CModioModCollectionEntry* CopyModioModCollectionEntry(const CModioModCollectionEntry* Item);

MODIODLL_EXPORT EModioModState GetModioModCollectionEntryModState(const CModioModCollectionEntry* ModioModCollectionEntry);
MODIODLL_EXPORT CModioModID* GetModioModCollectionEntryModID(const CModioModCollectionEntry* ModioModCollectionEntry);
MODIODLL_EXPORT CModioModInfo* GetModioModCollectionEntryModInfo(const CModioModCollectionEntry* ModioModCollectionEntry);
MODIODLL_EXPORT CModioString* GetModioModCollectionEntryPath(const CModioModCollectionEntry* ModioModCollectionEntry);
MODIODLL_EXPORT COptionalUInt64 GetModioModCollectionEntrySizeOnDisk(const CModioModCollectionEntry* ModioModCollectionEntry);

MODIODLL_EXPORT CModioImageList* CreateModioImageList();
MODIODLL_EXPORT void ReleaseModioImageList(CModioImageList* Item);
MODIODLL_EXPORT CModioImageList* CopyModioImageList(const CModioImageList* Item);
MODIODLL_EXPORT CModioImage* GetModioImageListImageByIndex(const CModioImageList* ModioImageList, uint64_t Index);
MODIODLL_EXPORT void  SetModioImageListImageByIndex(CModioImageList* ModioImageList, uint64_t Index, CModioImage const* Value);
MODIODLL_EXPORT uint64_t GetModioImageListCount(const CModioImageList* ModioImageList);
MODIODLL_EXPORT void SetModioImageListCount(CModioImageList* ModioImageList, uint64_t Count);


MODIODLL_EXPORT CModioErrorCode* CreateModioErrorCode();
MODIODLL_EXPORT void ReleaseModioErrorCode(CModioErrorCode* Item);
MODIODLL_EXPORT CModioErrorCode* CopyModioErrorCode(const CModioErrorCode* Item);
MODIODLL_EXPORT bool AreEqualModioErrorCode(const CModioErrorCode* First, const CModioErrorCode* Second);

MODIODLL_EXPORT CModioString* GetModioErrorCodeMessage(const CModioErrorCode* ModioErrorCode);
MODIODLL_EXPORT bool GetModioErrorCodeIsError(const CModioErrorCode* ModioErrorCode);

MODIODLL_EXPORT CModioAPIKey* CreateModioAPIKey(const char* Data, size_t Length);
MODIODLL_EXPORT void ReleaseModioAPIKey(CModioAPIKey* Item);
MODIODLL_EXPORT CModioAPIKey* CopyModioAPIKey(const CModioAPIKey* Item);


MODIODLL_EXPORT CModioModInfo* CreateModioModInfo();
MODIODLL_EXPORT void ReleaseModioModInfo(CModioModInfo* Item);
MODIODLL_EXPORT CModioModInfo* CopyModioModInfo(const CModioModInfo* Item);

MODIODLL_EXPORT CModioModID* GetModioModInfoModID(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioString* GetModioModInfoProfileName(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioString* GetModioModInfoProfileSummary(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioString* GetModioModInfoProfileDescription(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioString* GetModioModInfoProfileDescriptionPlaintext(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioString* GetModioModInfoProfileURL(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioUser* GetModioModInfoProfileSubmittedBy(const CModioModInfo* ModInfo);
MODIODLL_EXPORT int64_t GetModioModInfoProfileDateAdded(const CModioModInfo* ModInfo);
MODIODLL_EXPORT int64_t GetModioModInfoProfileDateUpdated(const CModioModInfo* ModInfo);
MODIODLL_EXPORT int64_t GetModioModInfoProfileDateLive(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioMaturityProfile GetModioModInfoProfileMaturityFlags(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioString* GetModioModInfoMetadataBlob(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioFileInfo* GetModioModInfoFileInfo(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioMetadataKVP* GetModioModInfoMetadataKVP(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioModTagList* GetModioModInfoTags(const CModioModInfo* ModInfo);
MODIODLL_EXPORT size_t GetModioModInfoNumGalleryImages(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioImageList* GetModioModInfoGallery(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioStringList* GetModioModInfoYoutubeURLs(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioStringList* GetModioModInfoSketchfabURLs(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioModStats* GetModioModInfoStats(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioModLogo* GetModioModInfoLogo(const CModioModInfo* ModInfo);
MODIODLL_EXPORT CModioString* GetModioModInfoVersion(const CModioModInfo* ModInfo);
MODIODLL_EXPORT EModioModServerSideStatus GetModioModInfoServerStatus(const CModioModInfo* ModInfo);
MODIODLL_EXPORT EModioObjectVisibility GetModioModInfoVisibility(const CModioModInfo* ModInfo);
MODIODLL_EXPORT uint64_t GetModioModInfoPrice(const CModioModInfo* ModInfo);
MODIODLL_EXPORT bool GetModioModInfoDependencies(const CModioModInfo* ModInfo);
MODIODLL_EXPORT void SetModioModInfoModID(CModioModInfo* Item, CModioModID const* ModID);
MODIODLL_EXPORT void SetModioModInfoProfileName(CModioModInfo* Item, CModioString const* ProfileName);
MODIODLL_EXPORT void SetModioModInfoProfileSummary(CModioModInfo* Item, CModioString const* ProfileSummary);
MODIODLL_EXPORT void SetModioModInfoProfileDescription(CModioModInfo* Item, CModioString const* ProfileDescription);
MODIODLL_EXPORT void SetModioModInfoProfileDescriptionPlaintext(CModioModInfo* Item, CModioString const* ProfileDescriptionPlaintext);
MODIODLL_EXPORT void SetModioModInfoProfileURL(CModioModInfo* Item, CModioString const* ProfileURL);
MODIODLL_EXPORT void SetModioModInfoProfileSubmittedBy(CModioModInfo* Item, CModioUser const* ProfileSubmittedBy);
MODIODLL_EXPORT void SetModioModInfoProfileDateAdded(CModioModInfo* Item, int64_t ProfileDateAdded);
MODIODLL_EXPORT void SetModioModInfoProfileDateUpdated(CModioModInfo* Item, int64_t ProfileDateUpdated);
MODIODLL_EXPORT void SetModioModInfoProfileDateLive(CModioModInfo* Item, int64_t ProfileDateLive);
MODIODLL_EXPORT void SetModioModInfoProfileMaturityFlags(CModioModInfo* Item, CModioMaturityProfile ProfileMaturityFlags);
MODIODLL_EXPORT void SetModioModInfoMetadataBlob(CModioModInfo* Item, CModioString const* MetadataBlob);
MODIODLL_EXPORT void SetModioModInfoFileInfo(CModioModInfo* Item, CModioFileInfo const* FileInfo);
MODIODLL_EXPORT void SetModioModInfoMetadataKVP(CModioModInfo* Item, CModioMetadataKVP const* MetadataKVP);
MODIODLL_EXPORT void SetModioModInfoTags(CModioModInfo* Item, CModioModTagList const* Tags);
MODIODLL_EXPORT void SetModioModInfoNumGalleryImages(CModioModInfo* Item, size_t NumGalleryImages);
MODIODLL_EXPORT void SetModioModInfoGallery(CModioModInfo* Item, CModioImageList const* Gallery);
MODIODLL_EXPORT void SetModioModInfoStats(CModioModInfo* Item, CModioModStats const* Stats);
MODIODLL_EXPORT void SetModioModInfoLogo(CModioModInfo* Item, CModioModLogo const* Logo);
MODIODLL_EXPORT void SetModioModInfoVersion(CModioModInfo* Item, CModioString const* Version);
MODIODLL_EXPORT void SetModioModInfoServerStatus(CModioModInfo* Item, EModioModServerSideStatus ServerStatus);
MODIODLL_EXPORT void SetModioModInfoVisibility(CModioModInfo* Item, EModioObjectVisibility Visibility);
MODIODLL_EXPORT void SetModioModInfoPrice(CModioModInfo* Item, uint64_t Price);
MODIODLL_EXPORT void SetModioModInfoDependencies(CModioModInfo* Item, bool Dependencies);

MODIODLL_EXPORT CModioImage* CreateModioImage();
MODIODLL_EXPORT void ReleaseModioImage(CModioImage* Item);
MODIODLL_EXPORT CModioImage* CopyModioImage(const CModioImage* Item);

MODIODLL_EXPORT CModioString* GetModioImageFilename(const CModioImage* ModioImage);
MODIODLL_EXPORT CModioString* GetModioImageOriginalURL(const CModioImage* ModioImage);
MODIODLL_EXPORT CModioString* GetModioImageThumb320x180(const CModioImage* ModioImage);
MODIODLL_EXPORT CModioString* GetModioImageThumb1280x720(const CModioImage* ModioImage);
MODIODLL_EXPORT void SetModioImageFilename(CModioImage* Item, CModioString const* Filename);
MODIODLL_EXPORT void SetModioImageOriginalURL(CModioImage* Item, CModioString const* OriginalURL);
MODIODLL_EXPORT void SetModioImageThumb320x180(CModioImage* Item, CModioString const* Thumb320x180);
MODIODLL_EXPORT void SetModioImageThumb1280x720(CModioImage* Item, CModioString const* Thumb1280x720);

MODIODLL_EXPORT CModioCreateModParams* CreateModioCreateModParams();
MODIODLL_EXPORT void ReleaseModioCreateModParams(CModioCreateModParams* Item);
MODIODLL_EXPORT CModioCreateModParams* CopyModioCreateModParams(const CModioCreateModParams* Item);

MODIODLL_EXPORT CModioString* GetModioCreateModParamsPathToLogoFile(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT CModioString* GetModioCreateModParamsName(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT CModioString* GetModioCreateModParamsSummary(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT CModioString* GetModioCreateModParamsNamePath(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT COptionalObjectVisibility GetModioCreateModParamsVisibility(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT CModioString* GetModioCreateModParamsDescription(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT CModioString* GetModioCreateModParamsHomepageURL(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT COptionalUInt64 GetModioCreateModParamsStock(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT COptionalModioMaturityProfile GetModioCreateModParamsMaturityRating(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT CModioString* GetModioCreateModParamsMetadataBlob(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT CModioStringList* GetModioCreateModParamsTags(const CModioCreateModParams* ModioCreateModParams);
MODIODLL_EXPORT void SetModioCreateModParamsPathToLogoFile(CModioCreateModParams* Item, CModioString const* PathToLogoFile);
MODIODLL_EXPORT void SetModioCreateModParamsName(CModioCreateModParams* Item, CModioString const* Name);
MODIODLL_EXPORT void SetModioCreateModParamsSummary(CModioCreateModParams* Item, CModioString const* Summary);
MODIODLL_EXPORT void SetModioCreateModParamsNamePath(CModioCreateModParams* Item, CModioString const* NamePath);
MODIODLL_EXPORT void SetModioCreateModParamsVisibility(CModioCreateModParams* Item, EModioObjectVisibility const* Visibility);
MODIODLL_EXPORT void SetModioCreateModParamsDescription(CModioCreateModParams* Item, CModioString const* Description);
MODIODLL_EXPORT void SetModioCreateModParamsHomepageURL(CModioCreateModParams* Item, CModioString const* HomepageURL);
MODIODLL_EXPORT void SetModioCreateModParamsStock(CModioCreateModParams* Item, size_t const* Stock);
MODIODLL_EXPORT void SetModioCreateModParamsMaturityRating(CModioCreateModParams* Item, CModioMaturityProfile const* MaturityRating);
MODIODLL_EXPORT void SetModioCreateModParamsMetadataBlob(CModioCreateModParams* Item, CModioString const* MetadataBlob);
MODIODLL_EXPORT void SetModioCreateModParamsTags(CModioCreateModParams* Item, CModioStringList const* Tags);

MODIODLL_EXPORT CModioPlatformList* CreateModioPlatformList();
MODIODLL_EXPORT void ReleaseModioPlatformList(CModioPlatformList* Item);
MODIODLL_EXPORT CModioPlatformList* CopyModioPlatformList(const CModioPlatformList* Item);
MODIODLL_EXPORT EModioModfilePlatform GetModioPlatformListPlatformByIndex(const CModioPlatformList* ModioPlatformList, uint64_t Index);
MODIODLL_EXPORT void  SetModioPlatformListPlatformByIndex(CModioPlatformList* ModioPlatformList, uint64_t Index, EModioModfilePlatform Value);
MODIODLL_EXPORT uint64_t GetModioPlatformListCount(const CModioPlatformList* ModioPlatformList);
MODIODLL_EXPORT void SetModioPlatformListCount(CModioPlatformList* ModioPlatformList, uint64_t Count);


MODIODLL_EXPORT CModioGamePlatformList* CreateModioGamePlatformList();
MODIODLL_EXPORT void ReleaseModioGamePlatformList(CModioGamePlatformList* Item);
MODIODLL_EXPORT CModioGamePlatformList* CopyModioGamePlatformList(const CModioGamePlatformList* Item);
MODIODLL_EXPORT CModioGamePlatform* GetModioGamePlatformListPlatformByIndex(const CModioGamePlatformList* ModioGamePlatformList, uint64_t Index);
MODIODLL_EXPORT void  SetModioGamePlatformListPlatformByIndex(CModioGamePlatformList* ModioGamePlatformList, uint64_t Index, CModioGamePlatform const* Value);
MODIODLL_EXPORT uint64_t GetModioGamePlatformListCount(const CModioGamePlatformList* ModioGamePlatformList);
MODIODLL_EXPORT void SetModioGamePlatformListCount(CModioGamePlatformList* ModioGamePlatformList, uint64_t Count);


MODIODLL_EXPORT CModioCreateModFileParams* CreateModioCreateModFileParams(const char* RootDirectoryPath, size_t RootDirectoryLength);
MODIODLL_EXPORT void ReleaseModioCreateModFileParams(CModioCreateModFileParams* Item);
MODIODLL_EXPORT CModioCreateModFileParams* CopyModioCreateModFileParams(const CModioCreateModFileParams* Item);

MODIODLL_EXPORT CModioString* GetModioCreateModFileParamsRootDirectory(const CModioCreateModFileParams* CreateParams);
MODIODLL_EXPORT CModioString* GetModioCreateModFileParamsVersion(const CModioCreateModFileParams* CreateParams);
MODIODLL_EXPORT CModioString* GetModioCreateModFileParamsChangelog(const CModioCreateModFileParams* CreateParams);
MODIODLL_EXPORT COptionalBool GetModioCreateModFileParamsSetAsActive(const CModioCreateModFileParams* CreateParams);
MODIODLL_EXPORT CModioString* GetModioCreateModFileParamsMetadataBlob(const CModioCreateModFileParams* CreateParams);
MODIODLL_EXPORT CModioPlatformList* GetModioCreateModFileParamsPlatforms(const CModioCreateModFileParams* CreateParams);
MODIODLL_EXPORT void SetModioCreateModFileParamsRootDirectory(CModioCreateModFileParams* Item, CModioString const* RootDirectory);
MODIODLL_EXPORT void SetModioCreateModFileParamsVersion(CModioCreateModFileParams* Item, CModioString const* Version);
MODIODLL_EXPORT void SetModioCreateModFileParamsChangelog(CModioCreateModFileParams* Item, CModioString const* Changelog);
MODIODLL_EXPORT void SetModioCreateModFileParamsSetAsActive(CModioCreateModFileParams* Item, bool const* SetAsActive);
MODIODLL_EXPORT void SetModioCreateModFileParamsMetadataBlob(CModioCreateModFileParams* Item, CModioString const* MetadataBlob);
MODIODLL_EXPORT void SetModioCreateModFileParamsPlatforms(CModioCreateModFileParams* Item, CModioPlatformList const* Platforms);

MODIODLL_EXPORT CModioAuthToken* CreateModioAuthToken();
MODIODLL_EXPORT void ReleaseModioAuthToken(CModioAuthToken* Item);
MODIODLL_EXPORT CModioAuthToken* CopyModioAuthToken(const CModioAuthToken* Item);

MODIODLL_EXPORT EAuthTokenState GetModioAuthTokenState(const CModioAuthToken* ModioAuthToken);
MODIODLL_EXPORT CModioString* GetModioAuthTokenRawToken(const CModioAuthToken* ModioAuthToken);

MODIODLL_EXPORT CModioModID* CreateModioModID(int64_t ID);
MODIODLL_EXPORT void ReleaseModioModID(CModioModID* Item);
MODIODLL_EXPORT CModioModID* CopyModioModID(const CModioModID* Item);
MODIODLL_EXPORT bool ModioModIDIsValid(const CModioModID* Item);
MODIODLL_EXPORT int64_t ModioModIDAsInteger(const CModioModID* Item);
MODIODLL_EXPORT bool AreEqualModioModID(const CModioModID* First, const CModioModID* Second);


MODIODLL_EXPORT CModioModIDList* CreateModioModIDList();
MODIODLL_EXPORT void ReleaseModioModIDList(CModioModIDList* Item);
MODIODLL_EXPORT CModioModIDList* CopyModioModIDList(const CModioModIDList* Item);
MODIODLL_EXPORT CModioModID* GetModioModIDListModByIndex(const CModioModIDList* ModIDList, uint64_t Index);
MODIODLL_EXPORT void  SetModioModIDListModByIndex(CModioModIDList* ModIDList, uint64_t Index, CModioModID const* Value);
MODIODLL_EXPORT uint64_t GetModioModIDListCount(const CModioModIDList* ModIDList);
MODIODLL_EXPORT void SetModioModIDListCount(CModioModIDList* ModIDList, uint64_t Count);


MODIODLL_EXPORT CModioUserID* CreateModioUserID(int64_t ID);
MODIODLL_EXPORT void ReleaseModioUserID(CModioUserID* Item);
MODIODLL_EXPORT CModioUserID* CopyModioUserID(const CModioUserID* Item);
MODIODLL_EXPORT bool ModioUserIDIsValid(const CModioUserID* Item);
MODIODLL_EXPORT int64_t ModioUserIDAsInteger(const CModioUserID* Item);
MODIODLL_EXPORT bool AreEqualModioUserID(const CModioUserID* First, const CModioUserID* Second);


MODIODLL_EXPORT CModioUserIDList* CreateModioUserIDList();
MODIODLL_EXPORT void ReleaseModioUserIDList(CModioUserIDList* Item);
MODIODLL_EXPORT CModioUserIDList* CopyModioUserIDList(const CModioUserIDList* Item);
MODIODLL_EXPORT CModioUserID* GetModioUserIDListUserByIndex(const CModioUserIDList* UserList, uint64_t Index);
MODIODLL_EXPORT void  SetModioUserIDListUserByIndex(CModioUserIDList* UserList, uint64_t Index, CModioUserID const* Value);
MODIODLL_EXPORT uint64_t GetModioUserIDListCount(const CModioUserIDList* UserList);
MODIODLL_EXPORT void SetModioUserIDListCount(CModioUserIDList* UserList, uint64_t Count);


MODIODLL_EXPORT CModioFileMetadataID* CreateModioFileMetadataID(int64_t ID);
MODIODLL_EXPORT void ReleaseModioFileMetadataID(CModioFileMetadataID* Item);
MODIODLL_EXPORT CModioFileMetadataID* CopyModioFileMetadataID(const CModioFileMetadataID* Item);
MODIODLL_EXPORT bool AreEqualModioFileMetadataID(const CModioFileMetadataID* First, const CModioFileMetadataID* Second);


MODIODLL_EXPORT CModioInitializeOptions* CreateModioInitializeOptions();
MODIODLL_EXPORT void ReleaseModioInitializeOptions(CModioInitializeOptions* Item);
MODIODLL_EXPORT CModioInitializeOptions* CopyModioInitializeOptions(const CModioInitializeOptions* Item);

MODIODLL_EXPORT CModioGameID* GetModioInitializeOptionsGameID(const CModioInitializeOptions* InitializeOptions);
MODIODLL_EXPORT CModioAPIKey* GetModioInitializeOptionsAPIKey(const CModioInitializeOptions* InitializeOptions);
MODIODLL_EXPORT CModioString* GetModioInitializeOptionsUserSession(const CModioInitializeOptions* InitializeOptions);
MODIODLL_EXPORT EModioPortal GetModioInitializeOptionsPortal(const CModioInitializeOptions* InitializeOptions);
MODIODLL_EXPORT EModioEnvironment GetModioInitializeOptionsEnvironment(const CModioInitializeOptions* InitializeOptions);
MODIODLL_EXPORT CModioStringMap* GetModioInitializeOptionsExtendedParameters(const CModioInitializeOptions* InitializeOptions);
MODIODLL_EXPORT void SetModioInitializeOptionsGameID(CModioInitializeOptions* Item, CModioGameID const* GameID);
MODIODLL_EXPORT void SetModioInitializeOptionsAPIKey(CModioInitializeOptions* Item, CModioAPIKey const* APIKey);
MODIODLL_EXPORT void SetModioInitializeOptionsUserSession(CModioInitializeOptions* Item, CModioString const* UserSession);
MODIODLL_EXPORT void SetModioInitializeOptionsPortal(CModioInitializeOptions* Item, EModioPortal Portal);
MODIODLL_EXPORT void SetModioInitializeOptionsEnvironment(CModioInitializeOptions* Item, EModioEnvironment Environment);
MODIODLL_EXPORT void SetModioInitializeOptionsExtendedParameters(CModioInitializeOptions* Item, CModioStringMap const* ExtendedParameters);

MODIODLL_EXPORT CModioGameID* CreateModioGameID(int64_t ID);
MODIODLL_EXPORT void ReleaseModioGameID(CModioGameID* Item);
MODIODLL_EXPORT CModioGameID* CopyModioGameID(const CModioGameID* Item);
MODIODLL_EXPORT bool ModioGameIDIsValid(const CModioGameID* Item);
MODIODLL_EXPORT int64_t ModioGameIDAsInteger(const CModioGameID* Item);
MODIODLL_EXPORT bool AreEqualModioGameID(const CModioGameID* First, const CModioGameID* Second);


MODIODLL_EXPORT void ReleaseModioReportParams(CModioReportParams* Item);
MODIODLL_EXPORT CModioReportParams* CopyModioReportParams(const CModioReportParams* Item);


MODIODLL_EXPORT CModioTransactionRecord* CreateModioTransactionRecord();
MODIODLL_EXPORT void ReleaseModioTransactionRecord(CModioTransactionRecord* Item);
MODIODLL_EXPORT CModioTransactionRecord* CopyModioTransactionRecord(const CModioTransactionRecord* Item);

MODIODLL_EXPORT CModioModID* GetModioTransactionRecordAssociatedMod(const CModioTransactionRecord* ModioTransactionRecord);
MODIODLL_EXPORT uint64_t GetModioTransactionRecordPrice(const CModioTransactionRecord* ModioTransactionRecord);
MODIODLL_EXPORT uint64_t GetModioTransactionRecordUpdatedUserWalletBalance(const CModioTransactionRecord* ModioTransactionRecord);
MODIODLL_EXPORT CModioModInfo* GetModioTransactionRecordMod(const CModioTransactionRecord* ModioTransactionRecord);
MODIODLL_EXPORT void SetModioTransactionRecordAssociatedMod(CModioTransactionRecord* Item, CModioModID const* AssociatedMod);
MODIODLL_EXPORT void SetModioTransactionRecordPrice(CModioTransactionRecord* Item, uint64_t Price);
MODIODLL_EXPORT void SetModioTransactionRecordUpdatedUserWalletBalance(CModioTransactionRecord* Item, uint64_t UpdatedUserWalletBalance);
MODIODLL_EXPORT void SetModioTransactionRecordMod(CModioTransactionRecord* Item, CModioModInfo const* Mod);

MODIODLL_EXPORT CModioEntitlementParams* CreateModioEntitlementParams();
MODIODLL_EXPORT void ReleaseModioEntitlementParams(CModioEntitlementParams* Item);
MODIODLL_EXPORT CModioEntitlementParams* CopyModioEntitlementParams(const CModioEntitlementParams* Item);

MODIODLL_EXPORT CModioStringMap* GetModioEntitlementParamsExtendedParameters(const CModioEntitlementParams* EntitlementParams);
MODIODLL_EXPORT void SetModioEntitlementParamsExtendedParameters(CModioEntitlementParams* Item, CModioStringMap const* ExtendedParameters);

MODIODLL_EXPORT CModioChangeMap* CreateModioChangeMap();
MODIODLL_EXPORT void ReleaseModioChangeMap(CModioChangeMap* Item);
MODIODLL_EXPORT CModioChangeMap* CopyModioChangeMap(const CModioChangeMap* Item);
MODIODLL_EXPORT void ModioChangeMapSetKeyValue(CModioChangeMap* Item, CModioModID const* Key, EModioChangeType Value);
MODIODLL_EXPORT EModioChangeType ModioChangeMapGetKeyValue(CModioChangeMap* Item, CModioModID const* Key);
MODIODLL_EXPORT CModioChangeMapIterator* ModioChangeMapBegin(CModioChangeMap* Item);
MODIODLL_EXPORT bool ModioChangeMapNext(CModioChangeMapIterator* Iterator);
MODIODLL_EXPORT CModioModID* ModioChangeMapGetKey(CModioChangeMapIterator* Iterator);
MODIODLL_EXPORT EModioChangeType ModioChangeMapGetValue(CModioChangeMapIterator* Iterator);
MODIODLL_EXPORT void ReleaseModioChangeMapIterator(CModioChangeMapIterator* Item);


MODIODLL_EXPORT CModioEntitlementConsumptionStatus* CreateModioEntitlementConsumptionStatus();
MODIODLL_EXPORT void ReleaseModioEntitlementConsumptionStatus(CModioEntitlementConsumptionStatus* Item);
MODIODLL_EXPORT CModioEntitlementConsumptionStatus* CopyModioEntitlementConsumptionStatus(const CModioEntitlementConsumptionStatus* Item);

MODIODLL_EXPORT CModioString* GetModioEntitlementConsumptionStatusTransactionId(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus);
MODIODLL_EXPORT EModioEntitlementConsumptionState GetModioEntitlementConsumptionStatusTransactionState(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus);
MODIODLL_EXPORT CModioString* GetModioEntitlementConsumptionStatusSkuId(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus);
MODIODLL_EXPORT bool GetModioEntitlementConsumptionStatusEntitlementConsumed(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus);
MODIODLL_EXPORT EModioEntitlementType GetModioEntitlementConsumptionStatusEntitlementType(const CModioEntitlementConsumptionStatus* EModioEntitlementType);
MODIODLL_EXPORT int32_t GetModioEntitlementConsumptionStatusTokensAllocated(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus);
MODIODLL_EXPORT bool GetModioEntitlementConsumptionStatusEntitlementRequiresRetry(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus);
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusTransactionId(CModioEntitlementConsumptionStatus* Item, CModioString const* TransactionId);
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusTransactionState(CModioEntitlementConsumptionStatus* Item, EModioEntitlementConsumptionState TransactionState);
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusSkuId(CModioEntitlementConsumptionStatus* Item, CModioString const* SkuId);
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusEntitlementConsumed(CModioEntitlementConsumptionStatus* Item, bool EntitlementConsumed);
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusEntitlementType(CModioEntitlementConsumptionStatus* Item, EModioEntitlementType EntitlementType);
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusTokensAllocated(CModioEntitlementConsumptionStatus* Item, int32_t TokensAllocated);

MODIODLL_EXPORT CModioEntitlementConsumptionStatusList* CreateModioEntitlementConsumptionStatusList();
MODIODLL_EXPORT void ReleaseModioEntitlementConsumptionStatusList(CModioEntitlementConsumptionStatusList* Item);
MODIODLL_EXPORT CModioEntitlementConsumptionStatusList* CopyModioEntitlementConsumptionStatusList(const CModioEntitlementConsumptionStatusList* Item);
MODIODLL_EXPORT CModioEntitlementConsumptionStatus* GetModioEntitlementConsumptionStatusListEntitlementByIndex(const CModioEntitlementConsumptionStatusList* ModioEntitlementList, uint64_t Index);
MODIODLL_EXPORT void  SetModioEntitlementConsumptionStatusListEntitlementByIndex(CModioEntitlementConsumptionStatusList* ModioEntitlementList, uint64_t Index, CModioEntitlementConsumptionStatus const* Value);
MODIODLL_EXPORT uint64_t GetModioEntitlementConsumptionStatusListCount(const CModioEntitlementConsumptionStatusList* ModioEntitlementList);
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusListCount(CModioEntitlementConsumptionStatusList* ModioEntitlementList, uint64_t Count);

MODIODLL_EXPORT CModioEntitlementConsumptionStatusList* GetModioEntitlementConsumptionStatusListEntitlementsThatRequireRetry(const CModioEntitlementConsumptionStatusList* EntitlementConsumptionStatus);

typedef void (*ModioErrorCodeCallback)(CModioErrorCode*, void* ContextPtr);
typedef void (*ModioGetModInfoCallback)(CModioErrorCode*, CModioModInfo*, void* ContextPtr);
typedef void (*ModioSubmitNewModCallback)(CModioErrorCode*, CModioModID*, void* ContextPtr);
typedef void (*ModioSubmitModChangesCallback)(CModioErrorCode*, CModioModInfo*, void* ContextPtr);
typedef void (*ModioGetModMediaCallback)(CModioErrorCode*, CModioString*, void* ContextPtr);
typedef void (*ModioModManagementCallback)(CModioModManagementEvent*, void* ContextPtr);
typedef void (*ModioLogCallback)(EModioLogLevel, CModioString*, void* ContextPtr);
typedef void (*ModioTermsOfUseCallback)(CModioErrorCode*, CModioTermsOfUse*, void* ContextPtr);
typedef void (*ModioModListCallback)(CModioErrorCode*, CModioModInfoList*, void* ContextPtr);
typedef void (*ModioUserListCallback)(CModioErrorCode*, CModioUserList*, void* ContextPtr);
typedef void (*ModioModTagOptionsCallback)(CModioErrorCode*, CModioModTagOptions*, void* ContextPtr);
typedef void (*ModioModDependenciesCallback)(CModioErrorCode*, CModioModDependencyList*, void* ContextPtr);
typedef void (*ModioTransactionCallback)(CModioErrorCode*, CModioTransactionRecord*, void* ContextPtr);
typedef void (*ModioWalletBalanceCallback)(CModioErrorCode*, uint64_t*, void* ContextPtr);
typedef void (*ModioPreviewExternalUpdatesCallback)(CModioErrorCode*, CModioChangeMap*, void* ContextPtr);
typedef void (*ModioUserEntitlementCallback)(CModioErrorCode*, CModioEntitlementConsumptionStatusList*, void* ContextPtr);

MODIODLL_EXPORT CModioErrorCode* ModioInitTempModSet(CModioModIDList* ModIds);
MODIODLL_EXPORT CModioErrorCode* ModioAddToTempModSet(CModioModIDList* ModIds);
MODIODLL_EXPORT CModioErrorCode* ModioRemoveFromTempModSet(CModioModIDList* ModIds);
MODIODLL_EXPORT CModioErrorCode* ModioCloseTempModSet();
MODIODLL_EXPORT CModioModCollectionMap* ModioQueryTempModSet();
MODIODLL_EXPORT void ModioInitializeAsync(CModioInitializeOptions* InitOptions, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioSetLogLevel(EModioLogLevel Level);
MODIODLL_EXPORT CModioModCreationHandle* ModioGetModCreationHandle();
MODIODLL_EXPORT void ModioSubscribeToModAsync(CModioModID* ModID, bool IncludeDependencies, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioSubmitNewModAsync(CModioModCreationHandle* Handle, CModioCreateModParams* Params, ModioSubmitNewModCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioUnsubscribeFromModAsync(CModioModID* ModID, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioFetchExternalUpdatesAsync(ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioPreviewExternalUpdatesAsync(ModioPreviewExternalUpdatesCallback Callback, void* ContextPtr);
MODIODLL_EXPORT CModioErrorCode* ModioEnableModManagement(ModioModManagementCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioDisableModManagement();
MODIODLL_EXPORT bool ModioIsModManagementBusy();
MODIODLL_EXPORT CModioErrorCode* ModioPrioritizeTransferForMod(CModioModID* ModID);
MODIODLL_EXPORT CModioModProgressInfo* ModioQueryCurrentModUpdate();
MODIODLL_EXPORT void ModioSetLogCallback(ModioLogCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioRunPendingHandlers();
MODIODLL_EXPORT CModioModCollectionMap* ModioQueryUserSubscriptions();
MODIODLL_EXPORT CModioModCollectionMap* ModioQueryUserInstallations(bool bIncludeOutdatedMods);
MODIODLL_EXPORT CModioModCollectionMap* ModioQuerySystemInstallations();
MODIODLL_EXPORT void ModioSubmitModRatingAsync(CModioModID* ModID, EModioRating Rating, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioGetModTagOptionsAsync(ModioModTagOptionsCallback Callback, void* ContextPtr);
MODIODLL_EXPORT CModioUser* ModioQueryUserProfile();
MODIODLL_EXPORT void ModioSetLanguage(EModioLanguage Locale);
MODIODLL_EXPORT void ModioShutdownAsync(ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioGetModInfoAsync(CModioModID* ModID, ModioGetModInfoCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioForceUninstallModAsync(CModioModID* ModToRemove, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioAuthenticateUserExternalAsync(CModioAuthenticationParams* User, EModioAuthenticationProvider Provider, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioVerifyUserAuthenticationAsync(ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioRefreshUserDataAsync(ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioGetModCreatorAvatarAsync(CModioModID* ModID, EModioAvatarSize AvatarSize, ModioGetModMediaCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioGetModLogoAsync(CModioModID* ModID, EModioLogoSize LogoSize, ModioGetModMediaCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioGetModGalleryImageAsync(CModioModID* ModID, EModioGallerySize GallerySize, uint32_t Index, ModioGetModMediaCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioGetTermsOfUseAsync(ModioTermsOfUseCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioSubmitModChangesAsync(CModioModID* ModID, CModioEditModParams* Params, ModioSubmitModChangesCallback Callback, void* ContextPtr);
MODIODLL_EXPORT bool ModioErrorCodeMatches(CModioErrorCode* ec, EModioErrorCondition Condition);
MODIODLL_EXPORT void ModioListAllModsAsync(CModioFilterParams* Filter, ModioModListCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioListUserCreatedModsAsync(CModioFilterParams* Filter, ModioModListCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioGetModDependenciesAsync(CModioModID* ModID, bool Recursive, ModioModDependenciesCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioRequestEmailAuthCodeAsync(CModioString* EmailAddress, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioAuthenticateUserEmailAsync(CModioString* EmailAddress, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioClearUserDataAsync(ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioGetUserMediaAsync(EModioAvatarSize AvatarSize, ModioGetModMediaCallback Callback, void* ContextPtr);
MODIODLL_EXPORT CModioValidationErrorList* ModioGetLastValidationError();
MODIODLL_EXPORT CModioReportParams* ModioCreateModioReportParamsForGame(CModioGameID* GameID, EModioReportType ReportType, CModioString* ReportDescription, CModioString* ReporterName, CModioString* ReporterContact);
MODIODLL_EXPORT CModioReportParams* ModioCreateModioReportParamsForUser(CModioUserID* UserID, EModioReportType ReportType, CModioString* ReportDescription, CModioString* ReporterName, CModioString* ReporterContact);
MODIODLL_EXPORT CModioReportParams* ModioCreateModioReportParamsForMod(CModioModID* ModID, EModioReportType ReportType, CModioString* ReportDescription, CModioString* ReporterName, CModioString* ReporterContact);
MODIODLL_EXPORT CModioStringList* ModioGetBaseModInstallationDirectories();
MODIODLL_EXPORT void ModioReportContentAsync(CModioReportParams* ReportParams, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioAddOrUpdateModLogoAsync(CModioModID* ModID, CModioString* NewLogoPath, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioArchiveModAsync(CModioModID* ModID, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioMuteUserAsync(CModioUserID* UserID, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioUnmuteUserAsync(CModioUserID* UserID, ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioGetMutedUsersAsync(ModioUserListCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioPurchaseModAsync(CModioModID* ModID, uint64_t ExpectedPrice, ModioTransactionCallback Callback, void* ContextPtr);
MODIODLL_EXPORT CModioString* ModioGetDefaultModInstallationDirectory(CModioGameID* GameID);
MODIODLL_EXPORT void ModioGetUserWalletBalanceAsync(ModioWalletBalanceCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioFetchUserPurchasesAsync(ModioErrorCodeCallback Callback, void* ContextPtr);
MODIODLL_EXPORT void ModioRefreshUserEntitlementsAsync(CModioEntitlementParams* Params, ModioUserEntitlementCallback Callback, void* ContextPtr);

#ifdef __cplusplus
}
#endif
