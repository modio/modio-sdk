
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/CInterface.h"
#include "modio/ModioSDK.h"
#include <vector>
#include <string>
#include <map>
#include <functional>

namespace Modio
{
	//Because the empty recursive case is no longer a template, we must declare it `inline`
	inline bool CheckNullImpl()
	{
		return true;
	}

	template<typename FirstArg, typename... ArgTypes>
	std::enable_if_t<std::is_pointer<FirstArg>::value, bool> CheckNullImpl(FirstArg&& First, ArgTypes&&... OtherArgs)
	{
		return First != nullptr && CheckNullImpl(OtherArgs...);
	}

	template<typename FirstArg, typename... ArgTypes>
	std::enable_if_t<!std::is_pointer<FirstArg>::value, bool> CheckNullImpl(FirstArg&& MODIO_UNUSED_ARGUMENT(First), ArgTypes&&... OtherArgs)
	{
		return CheckNullImpl(OtherArgs...);
	}

	template<typename... ArgTypes>
	bool CheckNull(ArgTypes... Args)
	{
		return CheckNullImpl(Args...);
	}

	template<typename... OtherArgs>
	void PostCallbackError(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
	{

		ModioAsio::post(
			Modio::Detail::Services::GetGlobalContext().get_executor(),
			[CompletionHandler = std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
				CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter), (OtherArgs {})...);
			});
		
	}

	template<typename WrappedCallbackType, typename... ArgTypes>
	bool RequireParametersNotNull(WrappedCallbackType&& Handler, ArgTypes&&... Args)
	{
		if (CheckNull(std::forward<ArgTypes>(Args)...))
		{
			return true;
		}
		else
		{
			PostCallbackError(std::forward<WrappedCallbackType>(Handler));
			return false;
		}
	}

	template<typename CApiType>
	auto& GetImpl(CApiType& Object)
	{
		return Object.Impl;
	}

	bool& GetImpl(bool& Object)
	{
		return Object;
	}

	template<typename CApiType>
	auto& GetImpl(CApiType* Object)
	{
		return Object->Impl;
	}

	template<typename CApiType>
	auto& GetListImpl(CApiType* Object)
	{
		return Object->Impl.GetRawList();
	}

	template<typename CApiType>
	auto& GetImpl(const CApiType* Object)
	{
		return Object->Impl;
	}

	template<typename CPPType, typename CApiType>
	CPPType ConstructFromImpl(CApiType&& Object)
	{
		return CPPType(GetImpl(Object));
	}

	// TODO: Consider type traits for specifying the implementation type?
	template<typename CApiType>
	void CModioSafeDelete(CApiType* Object)
	{
		if (Object != nullptr)
		{
			delete Object;
		}
	}

}


EModioLanguage ConvertEnumToC(Modio::Language InValue)
{
	return static_cast<EModioLanguage>(static_cast<std::underlying_type_t<Modio::Language>>(InValue));
}


EModioAuthenticationProvider ConvertEnumToC(Modio::AuthenticationProvider InValue)
{
	return static_cast<EModioAuthenticationProvider>(static_cast<std::underlying_type_t<Modio::AuthenticationProvider>>(InValue));
}


EModioSortFieldType ConvertEnumToC(Modio::FilterParams::SortFieldType InValue)
{
	return static_cast<EModioSortFieldType>(static_cast<std::underlying_type_t<Modio::FilterParams::SortFieldType>>(InValue));
}


EModioRevenueFilterType ConvertEnumToC(Modio::FilterParams::RevenueFilterType InValue)
{
	return static_cast<EModioRevenueFilterType>(static_cast<std::underlying_type_t<Modio::FilterParams::RevenueFilterType>>(InValue));
}


EModioSortDirection ConvertEnumToC(Modio::FilterParams::SortDirection InValue)
{
	return static_cast<EModioSortDirection>(static_cast<std::underlying_type_t<Modio::FilterParams::SortDirection>>(InValue));
}


EModioLogLevel ConvertEnumToC(Modio::LogLevel InValue)
{
	return static_cast<EModioLogLevel>(static_cast<std::underlying_type_t<Modio::LogLevel>>(InValue));
}


EModioMaturityOption ConvertEnumToC(Modio::MaturityOption InValue)
{
	return static_cast<EModioMaturityOption>(static_cast<std::underlying_type_t<Modio::MaturityOption>>(InValue));
}


EModioPortal ConvertEnumToC(Modio::Portal InValue)
{
	return static_cast<EModioPortal>(static_cast<std::underlying_type_t<Modio::Portal>>(InValue));
}


EModioEnvironment ConvertEnumToC(Modio::Environment InValue)
{
	return static_cast<EModioEnvironment>(static_cast<std::underlying_type_t<Modio::Environment>>(InValue));
}


EModioErrorCondition ConvertEnumToC(Modio::ErrorConditionTypes InValue)
{
	return static_cast<EModioErrorCondition>(static_cast<std::underlying_type_t<Modio::ErrorConditionTypes>>(InValue));
}


EModioVirusScanStatus ConvertEnumToC(Modio::FileMetadata::VirusScanStatus InValue)
{
	return static_cast<EModioVirusScanStatus>(static_cast<std::underlying_type_t<Modio::FileMetadata::VirusScanStatus>>(InValue));
}


EModioVirusStatus ConvertEnumToC(Modio::FileMetadata::VirusStatus InValue)
{
	return static_cast<EModioVirusStatus>(static_cast<std::underlying_type_t<Modio::FileMetadata::VirusStatus>>(InValue));
}


EModioModManagementEventType ConvertEnumToC(Modio::ModManagementEvent::EventType InValue)
{
	return static_cast<EModioModManagementEventType>(static_cast<std::underlying_type_t<Modio::ModManagementEvent::EventType>>(InValue));
}


EModioModServerSideStatus ConvertEnumToC(Modio::ModServerSideStatus InValue)
{
	return static_cast<EModioModServerSideStatus>(static_cast<std::underlying_type_t<Modio::ModServerSideStatus>>(InValue));
}


EModioModState ConvertEnumToC(Modio::ModState InValue)
{
	return static_cast<EModioModState>(static_cast<std::underlying_type_t<Modio::ModState>>(InValue));
}


EModioAvatarSize ConvertEnumToC(Modio::AvatarSize InValue)
{
	return static_cast<EModioAvatarSize>(static_cast<std::underlying_type_t<Modio::AvatarSize>>(InValue));
}


EAuthTokenState ConvertEnumToC(Modio::Detail::OAuthTokenState InValue)
{
	return static_cast<EAuthTokenState>(static_cast<std::underlying_type_t<Modio::Detail::OAuthTokenState>>(InValue));
}


EModioModfilePlatform ConvertEnumToC(Modio::ModfilePlatform InValue)
{
	return static_cast<EModioModfilePlatform>(static_cast<std::underlying_type_t<Modio::ModfilePlatform>>(InValue));
}


EModioLogoSize ConvertEnumToC(Modio::LogoSize InValue)
{
	return static_cast<EModioLogoSize>(static_cast<std::underlying_type_t<Modio::LogoSize>>(InValue));
}


EModioGallerySize ConvertEnumToC(Modio::GallerySize InValue)
{
	return static_cast<EModioGallerySize>(static_cast<std::underlying_type_t<Modio::GallerySize>>(InValue));
}


EModioRating ConvertEnumToC(Modio::Rating InValue)
{
	return static_cast<EModioRating>(static_cast<std::underlying_type_t<Modio::Rating>>(InValue));
}


EModioEntitlementConsumptionState ConvertEnumToC(Modio::EntitlementConsumptionState InValue)
{
	return static_cast<EModioEntitlementConsumptionState>(static_cast<std::underlying_type_t<Modio::EntitlementConsumptionState>>(InValue));
}


EModioEntitlementType ConvertEnumToC(Modio::EntitlementType InValue)
{
	return static_cast<EModioEntitlementType>(static_cast<std::underlying_type_t<Modio::EntitlementType>>(InValue));
}


EModioReportType ConvertEnumToC(Modio::ReportType InValue)
{
	return static_cast<EModioReportType>(static_cast<std::underlying_type_t<Modio::ReportType>>(InValue));
}


EModioChangeType ConvertEnumToC(Modio::UserSubscriptionList::ChangeType InValue)
{
	return static_cast<EModioChangeType>(static_cast<std::underlying_type_t<Modio::UserSubscriptionList::ChangeType>>(InValue));
}


EModioObjectVisibility ConvertEnumToC(Modio::ObjectVisibility InValue)
{
	return static_cast<EModioObjectVisibility>(static_cast<std::underlying_type_t<Modio::ObjectVisibility>>(InValue));
}


EModioGameCommunityOption ConvertEnumToC(Modio::GameCommunityOptions InValue)
{
	return static_cast<EModioGameCommunityOption>(static_cast<std::underlying_type_t<Modio::GameCommunityOptions>>(InValue));
}


EModioModCommunityOption ConvertEnumToC(Modio::ModCommunityOptions InValue)
{
	return static_cast<EModioModCommunityOption>(static_cast<std::underlying_type_t<Modio::ModCommunityOptions>>(InValue));
}


EModioGameMonetizationOption ConvertEnumToC(Modio::GameMonetizationOptions InValue)
{
	return static_cast<EModioGameMonetizationOption>(static_cast<std::underlying_type_t<Modio::GameMonetizationOptions>>(InValue));
}


struct CModioString 
{
	std::string Impl;
};

struct CModioTag 
{
	Modio::ModTag Impl;
};

struct CModioModManagementEvent 
{
	Modio::ModManagementEvent Impl;
};

struct CModioModCreationHandle 
{
	Modio::ModCreationHandle Impl;
};

struct CModioLink 
{
	Modio::Terms::Link Impl;
};

struct CModioEditModParams 
{
	Modio::EditModParams Impl;
};

struct CModioEditModCollectionParams 
{
	Modio::EditModCollectionParams Impl;
};

struct CModioIcon 
{
	Modio::Detail::Icon Impl;
};

struct CModioLogo 
{
	Modio::Detail::Logo Impl;
};

struct CModioHeaderImage 
{
	Modio::HeaderImage Impl;
};

struct CModioOtherUrl 
{
	Modio::OtherUrl Impl;
};

struct CModioOtherUrlList 
{
	std::vector<Modio::OtherUrl> Impl;
};

struct CModioTheme 
{
	Modio::Theme Impl;
};

struct CModioGameStats 
{
	Modio::GameStats Impl;
};

struct CModioTermsOfUse 
{
	Modio::Terms Impl;
};

struct CModioModTagList 
{
	std::vector<Modio::ModTag> Impl;
};

struct CModioModStats 
{
	Modio::ModStats Impl;
};

struct CModioModCollectionStats 
{
	Modio::ModCollectionStats Impl;
};

struct CModioGameInfo 
{
	Modio::GameInfo Impl;
};

struct CModioModLogo 
{
	Modio::Detail::Logo Impl;
};

struct CModioKVP 
{
	Modio::Metadata Impl;
};

struct CModioMetadataKVP 
{
	std::vector<Modio::Metadata> Impl;
};

struct CModioFileInfo 
{
	Modio::FileMetadata Impl;
};

struct CModioUser 
{
	Modio::User Impl;
};

struct CModioFilterParams 
{
	Modio::FilterParams Impl;
};

struct CModioAvatar 
{
	Modio::Detail::Avatar Impl;
};

struct CModioFieldError 
{
	Modio::FieldError Impl;
};

struct CModioValidationError 
{
	Modio::FieldError Impl;
};

struct CModioValidationErrorList 
{
	std::vector<Modio::FieldError> Impl;
};

struct CModioModProgressInfo 
{
	Modio::ModProgressInfo Impl;
};

struct CModioStringList 
{
	std::vector<std::string> Impl;
};

struct CModioStringMap 
{
	std::map<std::string, std::string> Impl;
};

struct CModioStringMapIterator 
{
	std::map<std::string, std::string>::iterator Iter;
	std::map<std::string, std::string>::iterator End;
};

struct CModioAuthenticationParams 
{
	Modio::AuthenticationParams Impl;
};

struct CModioModInfoList 
{
	Modio::List<std::vector, Modio::ModInfo> Impl;
};

struct CModioModCollectionInfoList 
{
	Modio::List<std::vector, Modio::ModCollectionInfo> Impl;
};

struct CModioUserList 
{
	Modio::List<std::vector, Modio::User> Impl;
};

struct CModioGamePlatform 
{
	Modio::GamePlatform Impl;
};

struct CModioModTagInfo 
{
	Modio::ModTagInfo Impl;
};

struct CModioModTagOptions 
{
	Modio::List<std::vector, Modio::ModTagInfo> Impl;
};

struct CModioModTagInfoList 
{
	std::vector<Modio::ModTagInfo> Impl;
};

struct CModioModCollectionMap 
{
	std::map<Modio::ModID, Modio::ModCollectionEntry> Impl;
};

struct CModioModCollectionMapIterator 
{
	std::map<Modio::ModID, Modio::ModCollectionEntry>::iterator Iter;
	std::map<Modio::ModID, Modio::ModCollectionEntry>::iterator End;
};

struct CModioModDependency 
{
	Modio::ModDependency Impl;
};

struct CModioModDependencyList 
{
	Modio::ModDependencyList Impl;
};

struct CModioModCollectionEntry 
{
	Modio::ModCollectionEntry Impl;
};

struct CModioImageList 
{
	Modio::List<std::vector, Modio::Detail::Image> Impl;
};

struct CModioErrorCode 
{
	Modio::ErrorCode Impl;
};

struct CModioAPIKey 
{
	Modio::ApiKey Impl;
};

struct CModioModInfo 
{
	Modio::ModInfo Impl;
};

struct CModioModCollectionInfo 
{
	Modio::ModCollectionInfo Impl;
};

struct CModioImage 
{
	Modio::Detail::Image Impl;
};

struct CModioCreateModCollectionParams 
{
	Modio::CreateModCollectionParams Impl;
};

struct CModioCreateModParams 
{
	Modio::CreateModParams Impl;
};

struct CModioPlatformList 
{
	std::vector<Modio::ModfilePlatform> Impl;
};

struct CModioGamePlatformList 
{
	std::vector<Modio::GamePlatform> Impl;
};

struct CModioCreateModFileParams 
{
	Modio::CreateModFileParams Impl;
};

struct CModioAuthToken 
{
	Modio::Detail::OAuthToken Impl;
};

struct CModioModID 
{
	Modio::ModID Impl;
};

struct CModioModCollectionID 
{
	Modio::ModCollectionID Impl;
};

struct CModioModIDList 
{
	std::vector<Modio::ModID> Impl;
};

struct CModioUserID 
{
	Modio::UserID Impl;
};

struct CModioUserIDList 
{
	std::vector<Modio::UserID> Impl;
};

struct CModioFileMetadataID 
{
	Modio::FileMetadataID Impl;
};

struct CModioInitializeOptions 
{
	Modio::InitializeOptions Impl;
};

struct CModioGameID 
{
	Modio::GameID Impl;
};

struct CModioReportParams 
{
	Modio::ReportParams Impl;
};

struct CModioTransactionRecord 
{
	Modio::TransactionRecord Impl;
};

struct CModioEntitlementParams 
{
	Modio::EntitlementParams Impl;
};

struct CModioChangeMap 
{
	std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType> Impl;
};

struct CModioChangeMapIterator 
{
	std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType>::iterator Iter;
	std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType>::iterator End;
};

struct CModioEntitlementConsumptionStatus 
{
	Modio::EntitlementConsumptionStatus Impl;
};

struct CModioEntitlementConsumptionStatusList 
{
	Modio::EntitlementConsumptionStatusList Impl;
};

struct CModioEntitlementList 
{
	Modio::EntitlementList Impl;
};

struct CModioEntitlement 
{
	Modio::Entitlement Impl;
};


MODIODLL_EXPORT CModioString* CreateModioString(const char* Data, size_t Length)
{
return new CModioString{ std::string { Data, Length } };
}

MODIODLL_EXPORT void ReleaseModioString(CModioString* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioString* CopyModioString(const CModioString* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioString { *Item };
}

MODIODLL_EXPORT const char* GetModioStringData(const CModioString* ModioString)
{
	return ModioString->Impl.data();
}

MODIODLL_EXPORT size_t GetModioStringLength(const CModioString* ModioString)
{
	return size_t{ ModioString->Impl.size() };
}



MODIODLL_EXPORT CModioTag* CreateModioTag()
{
	return new CModioTag{ Modio::ModTag {} };
}

MODIODLL_EXPORT void ReleaseModioTag(CModioTag* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioTag* CopyModioTag(const CModioTag* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioTag { *Item };
}

MODIODLL_EXPORT CModioString* GetModioTagTag(const CModioTag* ModioTag)
{
	return new CModioString{ ModioTag->Impl.Tag };
}

MODIODLL_EXPORT CModioString* GetModioTagTagLocalized(const CModioTag* ModioTag)
{
	return new CModioString{ ModioTag->Impl.TagLocalized };
}


MODIODLL_EXPORT void SetModioTagTag(CModioTag* Item, const CModioString* Tag)
{
	if (Item != nullptr)
	{
		Item->Impl.Tag = Modio::GetImpl(Tag);
	}
}
MODIODLL_EXPORT void SetModioTagTagLocalized(CModioTag* Item, const CModioString* TagLocalized)
{
	if (Item != nullptr)
	{
		Item->Impl.TagLocalized = Modio::GetImpl(TagLocalized);
	}
}

MODIODLL_EXPORT CModioModManagementEvent* CreateModioModManagementEvent()
{
	return new CModioModManagementEvent{ Modio::ModManagementEvent {} };
}

MODIODLL_EXPORT void ReleaseModioModManagementEvent(CModioModManagementEvent* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModManagementEvent* CopyModioModManagementEvent(const CModioModManagementEvent* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModManagementEvent { *Item };
}

MODIODLL_EXPORT CModioModID* GetModioModManagementEventModID(const CModioModManagementEvent* ModManagementEvent)
{
	return new CModioModID{ ModManagementEvent->Impl.ID };
}

MODIODLL_EXPORT EModioModManagementEventType GetModioModManagementEventEventType(const CModioModManagementEvent* ModManagementEvent)
{
	return ConvertEnumToC( ModManagementEvent->Impl.Event );
}

MODIODLL_EXPORT CModioErrorCode* GetModioModManagementEventStatus(const CModioModManagementEvent* ModManagementEvent)
{
	return new CModioErrorCode{ ModManagementEvent->Impl.Status };
}


MODIODLL_EXPORT void SetModioModManagementEventModID(CModioModManagementEvent* Item, const CModioModID* ModID)
{
	if (Item != nullptr)
	{
		Item->Impl.ID = Modio::GetImpl(ModID);
	}
}
MODIODLL_EXPORT void SetModioModManagementEventEventType(CModioModManagementEvent* Item, const EModioModManagementEventType EventType)
{
	if (Item != nullptr)
	{
		Item->Impl.Event = static_cast<Modio::ModManagementEvent::EventType>(EventType);
	}
}
MODIODLL_EXPORT void SetModioModManagementEventStatus(CModioModManagementEvent* Item, const CModioErrorCode* Status)
{
	if (Item != nullptr)
	{
		Item->Impl.Status = Modio::GetImpl(Status);
	}
}

MODIODLL_EXPORT CModioModCreationHandle* CreateModioModCreationHandle()
{
	return new CModioModCreationHandle{ Modio::ModCreationHandle {} };
}

MODIODLL_EXPORT void ReleaseModioModCreationHandle(CModioModCreationHandle* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModCreationHandle* CopyModioModCreationHandle(const CModioModCreationHandle* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModCreationHandle { *Item };
}

MODIODLL_EXPORT bool AreEqualModioModCreationHandle(const CModioModCreationHandle* First, const CModioModCreationHandle* Second)
{
	return Modio::GetImpl(First) == Modio::GetImpl(Second);
}



MODIODLL_EXPORT CModioLink* CreateModioLink()
{
	return new CModioLink{ Modio::Terms::Link {} };
}

MODIODLL_EXPORT void ReleaseModioLink(CModioLink* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioLink* CopyModioLink(const CModioLink* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioLink { *Item };
}

MODIODLL_EXPORT CModioString* GetModioLinkText(const CModioLink* ModioLink)
{
	return new CModioString{ ModioLink->Impl.Text };
}

MODIODLL_EXPORT CModioString* GetModioLinkURL(const CModioLink* ModioLink)
{
	return new CModioString{ ModioLink->Impl.URL };
}

MODIODLL_EXPORT bool GetModioLinkRequired(const CModioLink* ModioLink)
{
	return bool{ ModioLink->Impl.bRequired };
}


MODIODLL_EXPORT void SetModioLinkText(CModioLink* Item, const CModioString* Text)
{
	if (Item != nullptr)
	{
		Item->Impl.Text = Modio::GetImpl(Text);
	}
}
MODIODLL_EXPORT void SetModioLinkURL(CModioLink* Item, const CModioString* URL)
{
	if (Item != nullptr)
	{
		Item->Impl.URL = Modio::GetImpl(URL);
	}
}
MODIODLL_EXPORT void SetModioLinkRequired(CModioLink* Item, const bool Required)
{
	if (Item != nullptr)
	{
		Item->Impl.bRequired = Required;
	}
}

MODIODLL_EXPORT CModioEditModParams* CreateModioEditModParams()
{
	return new CModioEditModParams{ Modio::EditModParams {} };
}

MODIODLL_EXPORT void ReleaseModioEditModParams(CModioEditModParams* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioEditModParams* CopyModioEditModParams(const CModioEditModParams* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioEditModParams { *Item };
}

MODIODLL_EXPORT CModioString* GetModioEditModParamsName(const CModioEditModParams* ModioEditModParams)
{
	if (ModioEditModParams->Impl.Name.has_value())
	{
		return new CModioString{ *(ModioEditModParams->Impl.Name) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModParamsSummary(const CModioEditModParams* ModioEditModParams)
{
	if (ModioEditModParams->Impl.Summary.has_value())
	{
		return new CModioString{ *(ModioEditModParams->Impl.Summary) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModParamsNamePath(const CModioEditModParams* ModioEditModParams)
{
	if (ModioEditModParams->Impl.NamePath.has_value())
	{
		return new CModioString{ *(ModioEditModParams->Impl.NamePath) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT COptionalObjectVisibility GetModioEditModParamsVisibility(const CModioEditModParams* ModioEditModParams)
{
	if (ModioEditModParams->Impl.Visibility.has_value())
	{
		return { ConvertEnumToC( *(ModioEditModParams->Impl.Visibility) ), true };
	}
	else
	{
		return { ConvertEnumToC(Modio::ObjectVisibility::Hidden), false };
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModParamsDescription(const CModioEditModParams* ModioEditModParams)
{
	if (ModioEditModParams->Impl.Description.has_value())
	{
		return new CModioString{ *(ModioEditModParams->Impl.Description) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModParamsHomepageURL(const CModioEditModParams* ModioEditModParams)
{
	if (ModioEditModParams->Impl.HomepageURL.has_value())
	{
		return new CModioString{ *(ModioEditModParams->Impl.HomepageURL) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT COptionalModioMaturityProfile GetModioEditModParamsMaturityRating(const CModioEditModParams* ModioEditModParams)
{
	if (ModioEditModParams->Impl.MaturityRating.has_value())
	{
		return { ConvertEnumToC(static_cast<Modio::MaturityOption>(*(ModioEditModParams->Impl.MaturityRating))), true };
	}
	else
	{
		return { ConvertEnumToC(Modio::MaturityOption::None), false };
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModParamsMetadataBlob(const CModioEditModParams* ModioEditModParams)
{
	if (ModioEditModParams->Impl.MetadataBlob.has_value())
	{
		return new CModioString{ *(ModioEditModParams->Impl.MetadataBlob) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModParamsLogoPath(const CModioEditModParams* ModioEditModParams)
{
	if (ModioEditModParams->Impl.LogoPath.has_value())
	{
		return new CModioString{ *(ModioEditModParams->Impl.LogoPath) };
	}
	else
	{
		return nullptr;
	}
}


MODIODLL_EXPORT void SetModioEditModParamsName(CModioEditModParams* Item, const CModioString* Name)
{
	if (Item != nullptr)
	{
		if(Name)
		{
			Item->Impl.Name = Modio::GetImpl(Name);
		}
		else if(Item->Impl.Name.has_value())
		{
			Item->Impl.Name.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModParamsSummary(CModioEditModParams* Item, const CModioString* Summary)
{
	if (Item != nullptr)
	{
		if(Summary)
		{
			Item->Impl.Summary = Modio::GetImpl(Summary);
		}
		else if(Item->Impl.Summary.has_value())
		{
			Item->Impl.Summary.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModParamsNamePath(CModioEditModParams* Item, const CModioString* NamePath)
{
	if (Item != nullptr)
	{
		if(NamePath)
		{
			Item->Impl.NamePath = Modio::GetImpl(NamePath);
		}
		else if(Item->Impl.NamePath.has_value())
		{
			Item->Impl.NamePath.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModParamsDescription(CModioEditModParams* Item, const CModioString* Description)
{
	if (Item != nullptr)
	{
		if(Description)
		{
			Item->Impl.Description = Modio::GetImpl(Description);
		}
		else if(Item->Impl.Description.has_value())
		{
			Item->Impl.Description.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModParamsHomepageURL(CModioEditModParams* Item, const CModioString* HomepageURL)
{
	if (Item != nullptr)
	{
		if(HomepageURL)
		{
			Item->Impl.HomepageURL = Modio::GetImpl(HomepageURL);
		}
		else if(Item->Impl.HomepageURL.has_value())
		{
			Item->Impl.HomepageURL.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModParamsMaturityRating(CModioEditModParams* Item, CModioMaturityProfile const* MaturityRating)
{
	if (Item != nullptr)
	{
		if(MaturityRating)
		{
			Item->Impl.MaturityRating = static_cast<Modio::MaturityOption>(*MaturityRating);
		}
		else if(Item->Impl.MaturityRating.has_value())
		{
			Item->Impl.MaturityRating.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModParamsMetadataBlob(CModioEditModParams* Item, const CModioString* MetadataBlob)
{
	if (Item != nullptr)
	{
		if(MetadataBlob)
		{
			Item->Impl.MetadataBlob = Modio::GetImpl(MetadataBlob);
		}
		else if(Item->Impl.MetadataBlob.has_value())
		{
			Item->Impl.MetadataBlob.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModParamsLogoPath(CModioEditModParams* Item, const CModioString* LogoPath)
{
	if (Item != nullptr)
	{
		if(LogoPath)
		{
			Item->Impl.LogoPath = Modio::GetImpl(LogoPath);
		}
		else if(Item->Impl.LogoPath.has_value())
		{
			Item->Impl.LogoPath.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModParamsVisibility(CModioEditModParams* Item, const EModioObjectVisibility* Visibility)
{
	if (Item != nullptr)
	{
		if(Visibility)
		{
			Item->Impl.Visibility = static_cast<Modio::ObjectVisibility>(*Visibility);
		}
		else if(Item->Impl.Visibility.has_value())
		{
			Item->Impl.Visibility.reset();
		}
	}
}

MODIODLL_EXPORT CModioEditModCollectionParams* CreateModioEditModCollectionParams()
{
	return new CModioEditModCollectionParams{ Modio::EditModCollectionParams {} };
}

MODIODLL_EXPORT void ReleaseModioEditModCollectionParams(CModioEditModCollectionParams* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioEditModCollectionParams* CopyModioEditModCollectionParams(const CModioEditModCollectionParams* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioEditModCollectionParams { *Item };
}

MODIODLL_EXPORT CModioString* GetModioEditModCollectionParamsLogoPath(const CModioEditModCollectionParams* ModioEditModCollectionParams)
{
	if (ModioEditModCollectionParams->Impl.LogoPath.has_value())
	{
		return new CModioString{ *(ModioEditModCollectionParams->Impl.LogoPath) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModCollectionParamsName(const CModioEditModCollectionParams* ModioEditModCollectionParams)
{
	if (ModioEditModCollectionParams->Impl.Name.has_value())
	{
		return new CModioString{ *(ModioEditModCollectionParams->Impl.Name) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModCollectionParamsSummary(const CModioEditModCollectionParams* ModioEditModCollectionParams)
{
	if (ModioEditModCollectionParams->Impl.Summary.has_value())
	{
		return new CModioString{ *(ModioEditModCollectionParams->Impl.Summary) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModCollectionParamsCategory(const CModioEditModCollectionParams* ModioEditModCollectionParams)
{
	if (ModioEditModCollectionParams->Impl.Category.has_value())
	{
		return new CModioString{ *(ModioEditModCollectionParams->Impl.Category) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT COptionalObjectVisibility GetModioEditModCollectionParamsVisibility(const CModioEditModCollectionParams* ModioEditModCollectionParams)
{
	if (ModioEditModCollectionParams->Impl.Visibility.has_value())
	{
		return { ConvertEnumToC( *(ModioEditModCollectionParams->Impl.Visibility) ), true };
	}
	else
	{
		return { ConvertEnumToC(Modio::ObjectVisibility::Hidden), false };
	}
}

MODIODLL_EXPORT CModioModIDList* GetModioEditModCollectionParamsMods(const CModioEditModCollectionParams* ModioEditModCollectionParams)
{
	if (ModioEditModCollectionParams->Impl.Mods.has_value())
	{
		return new CModioModIDList{ *(ModioEditModCollectionParams->Impl.Mods) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModCollectionParamsNamePath(const CModioEditModCollectionParams* ModioEditModCollectionParams)
{
	if (ModioEditModCollectionParams->Impl.NamePath.has_value())
	{
		return new CModioString{ *(ModioEditModCollectionParams->Impl.NamePath) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioEditModCollectionParamsDescription(const CModioEditModCollectionParams* ModioEditModCollectionParams)
{
	if (ModioEditModCollectionParams->Impl.Description.has_value())
	{
		return new CModioString{ *(ModioEditModCollectionParams->Impl.Description) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioStringList* GetModioEditModCollectionParamsTags(const CModioEditModCollectionParams* ModioEditModCollectionParams)
{
	if (ModioEditModCollectionParams->Impl.Tags.has_value())
	{
		return new CModioStringList{ *(ModioEditModCollectionParams->Impl.Tags) };
	}
	else
	{
		return nullptr;
	}
}


MODIODLL_EXPORT void SetModioEditModCollectionParamsLogoPath(CModioEditModCollectionParams* Item, const CModioString* LogoPath)
{
	if (Item != nullptr)
	{
		if(LogoPath)
		{
			Item->Impl.LogoPath = Modio::GetImpl(LogoPath);
		}
		else if(Item->Impl.LogoPath.has_value())
		{
			Item->Impl.LogoPath.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModCollectionParamsName(CModioEditModCollectionParams* Item, const CModioString* Name)
{
	if (Item != nullptr)
	{
		if(Name)
		{
			Item->Impl.Name = Modio::GetImpl(Name);
		}
		else if(Item->Impl.Name.has_value())
		{
			Item->Impl.Name.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModCollectionParamsSummary(CModioEditModCollectionParams* Item, const CModioString* Summary)
{
	if (Item != nullptr)
	{
		if(Summary)
		{
			Item->Impl.Summary = Modio::GetImpl(Summary);
		}
		else if(Item->Impl.Summary.has_value())
		{
			Item->Impl.Summary.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModCollectionParamsCategory(CModioEditModCollectionParams* Item, const CModioString* Category)
{
	if (Item != nullptr)
	{
		if(Category)
		{
			Item->Impl.Category = Modio::GetImpl(Category);
		}
		else if(Item->Impl.Category.has_value())
		{
			Item->Impl.Category.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModCollectionParamsVisibility(CModioEditModCollectionParams* Item, const EModioObjectVisibility* Visibility)
{
	if (Item != nullptr)
	{
		if(Visibility)
		{
			Item->Impl.Visibility = static_cast<Modio::ObjectVisibility>(*Visibility);
		}
		else if(Item->Impl.Visibility.has_value())
		{
			Item->Impl.Visibility.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModCollectionParamsMods(CModioEditModCollectionParams* Item, const CModioModIDList* Mods)
{
	if (Item != nullptr)
	{
		if(Mods)
		{
			Item->Impl.Mods = Modio::GetImpl(Mods);
		}
		else if(Item->Impl.Mods.has_value())
		{
			Item->Impl.Mods.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModCollectionParamsNamePath(CModioEditModCollectionParams* Item, const CModioString* NamePath)
{
	if (Item != nullptr)
	{
		if(NamePath)
		{
			Item->Impl.NamePath = Modio::GetImpl(NamePath);
		}
		else if(Item->Impl.NamePath.has_value())
		{
			Item->Impl.NamePath.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModCollectionParamsDescription(CModioEditModCollectionParams* Item, const CModioString* Description)
{
	if (Item != nullptr)
	{
		if(Description)
		{
			Item->Impl.Description = Modio::GetImpl(Description);
		}
		else if(Item->Impl.Description.has_value())
		{
			Item->Impl.Description.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioEditModCollectionParamsTags(CModioEditModCollectionParams* Item, const CModioStringList* Tags)
{
	if (Item != nullptr)
	{
		if(Tags)
		{
			Item->Impl.Tags = Modio::GetImpl(Tags);
		}
		else if(Item->Impl.Tags.has_value())
		{
			Item->Impl.Tags.reset();
		}
	}
}

MODIODLL_EXPORT CModioIcon* CreateModioIcon()
{
	return new CModioIcon{ Modio::Detail::Icon {} };
}

MODIODLL_EXPORT void ReleaseModioIcon(CModioIcon* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioIcon* CopyModioIcon(const CModioIcon* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioIcon { *Item };
}

MODIODLL_EXPORT CModioString* GetModioIconFilename(const CModioIcon* ModioIcon)
{
	return new CModioString{ ModioIcon->Impl.Filename };
}

MODIODLL_EXPORT CModioString* GetModioIconOriginal(const CModioIcon* ModioIcon)
{
	return new CModioString{ ModioIcon->Impl.Original };
}

MODIODLL_EXPORT CModioString* GetModioIconThumb64(const CModioIcon* ModioIcon)
{
	return new CModioString{ ModioIcon->Impl.Thumb64x64 };
}

MODIODLL_EXPORT CModioString* GetModioIconThumb128(const CModioIcon* ModioIcon)
{
	return new CModioString{ ModioIcon->Impl.Thumb128x128 };
}

MODIODLL_EXPORT CModioString* GetModioIconThumb256(const CModioIcon* ModioIcon)
{
	return new CModioString{ ModioIcon->Impl.Thumb256x256 };
}


MODIODLL_EXPORT void SetModioIconFilename(CModioIcon* Item, const CModioString* Filename)
{
	if (Item != nullptr)
	{
		Item->Impl.Filename = Modio::GetImpl(Filename);
	}
}
MODIODLL_EXPORT void SetModioIconOriginal(CModioIcon* Item, const CModioString* Original)
{
	if (Item != nullptr)
	{
		Item->Impl.Original = Modio::GetImpl(Original);
	}
}
MODIODLL_EXPORT void SetModioIconThumb64(CModioIcon* Item, const CModioString* Thumb64)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb64x64 = Modio::GetImpl(Thumb64);
	}
}
MODIODLL_EXPORT void SetModioIconThumb128(CModioIcon* Item, const CModioString* Thumb128)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb128x128 = Modio::GetImpl(Thumb128);
	}
}
MODIODLL_EXPORT void SetModioIconThumb256(CModioIcon* Item, const CModioString* Thumb256)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb256x256 = Modio::GetImpl(Thumb256);
	}
}

MODIODLL_EXPORT CModioLogo* CreateModioLogo()
{
	return new CModioLogo{ Modio::Detail::Logo {} };
}

MODIODLL_EXPORT void ReleaseModioLogo(CModioLogo* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioLogo* CopyModioLogo(const CModioLogo* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioLogo { *Item };
}

MODIODLL_EXPORT CModioString* GetModioLogoFilename(const CModioLogo* ModioLogo)
{
	return new CModioString{ ModioLogo->Impl.Filename };
}

MODIODLL_EXPORT CModioString* GetModioLogoOriginal(const CModioLogo* ModioLogo)
{
	return new CModioString{ ModioLogo->Impl.Original };
}

MODIODLL_EXPORT CModioString* GetModioLogoThumb320(const CModioLogo* ModioLogo)
{
	return new CModioString{ ModioLogo->Impl.Thumb320x180 };
}

MODIODLL_EXPORT CModioString* GetModioLogoThumb640(const CModioLogo* ModioLogo)
{
	return new CModioString{ ModioLogo->Impl.Thumb640x360 };
}

MODIODLL_EXPORT CModioString* GetModioLogoThumb1280(const CModioLogo* ModioLogo)
{
	return new CModioString{ ModioLogo->Impl.Thumb1280x720 };
}


MODIODLL_EXPORT void SetModioLogoFilename(CModioLogo* Item, const CModioString* Filename)
{
	if (Item != nullptr)
	{
		Item->Impl.Filename = Modio::GetImpl(Filename);
	}
}
MODIODLL_EXPORT void SetModioLogoOriginal(CModioLogo* Item, const CModioString* Original)
{
	if (Item != nullptr)
	{
		Item->Impl.Original = Modio::GetImpl(Original);
	}
}
MODIODLL_EXPORT void SetModioLogoThumb320(CModioLogo* Item, const CModioString* Thumb320)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb320x180 = Modio::GetImpl(Thumb320);
	}
}
MODIODLL_EXPORT void SetModioLogoThumb640(CModioLogo* Item, const CModioString* Thumb640)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb640x360 = Modio::GetImpl(Thumb640);
	}
}
MODIODLL_EXPORT void SetModioLogoThumb1280(CModioLogo* Item, const CModioString* Thumb1280)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb1280x720 = Modio::GetImpl(Thumb1280);
	}
}

MODIODLL_EXPORT CModioHeaderImage* CreateModioHeaderImage()
{
	return new CModioHeaderImage{ Modio::HeaderImage {} };
}

MODIODLL_EXPORT void ReleaseModioHeaderImage(CModioHeaderImage* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioHeaderImage* CopyModioHeaderImage(const CModioHeaderImage* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioHeaderImage { *Item };
}

MODIODLL_EXPORT CModioString* GetModioHeaderImageFilename(const CModioHeaderImage* HeaderImage)
{
	return new CModioString{ HeaderImage->Impl.Filename };
}

MODIODLL_EXPORT CModioString* GetModioHeaderImageOriginal(const CModioHeaderImage* HeaderImage)
{
	return new CModioString{ HeaderImage->Impl.Original };
}


MODIODLL_EXPORT void SetModioHeaderImageFilename(CModioHeaderImage* Item, const CModioString* Filename)
{
	if (Item != nullptr)
	{
		Item->Impl.Filename = Modio::GetImpl(Filename);
	}
}
MODIODLL_EXPORT void SetModioHeaderImageOriginal(CModioHeaderImage* Item, const CModioString* Original)
{
	if (Item != nullptr)
	{
		Item->Impl.Original = Modio::GetImpl(Original);
	}
}

MODIODLL_EXPORT CModioOtherUrl* CreateModioOtherUrl()
{
	return new CModioOtherUrl{ Modio::OtherUrl {} };
}

MODIODLL_EXPORT void ReleaseModioOtherUrl(CModioOtherUrl* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioOtherUrl* CopyModioOtherUrl(const CModioOtherUrl* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioOtherUrl { *Item };
}

MODIODLL_EXPORT CModioString* GetModioOtherUrlLabel(const CModioOtherUrl* OtherUrl)
{
	return new CModioString{ OtherUrl->Impl.Label };
}

MODIODLL_EXPORT CModioString* GetModioOtherUrlUrl(const CModioOtherUrl* OtherUrl)
{
	return new CModioString{ OtherUrl->Impl.Url };
}


MODIODLL_EXPORT void SetModioOtherUrlLabel(CModioOtherUrl* Item, const CModioString* Label)
{
	if (Item != nullptr)
	{
		Item->Impl.Label = Modio::GetImpl(Label);
	}
}
MODIODLL_EXPORT void SetModioOtherUrlUrl(CModioOtherUrl* Item, const CModioString* Url)
{
	if (Item != nullptr)
	{
		Item->Impl.Url = Modio::GetImpl(Url);
	}
}

MODIODLL_EXPORT CModioOtherUrlList* CreateModioOtherUrlList()
{
	return new CModioOtherUrlList{ std::vector<Modio::OtherUrl> {} };
}

MODIODLL_EXPORT void ReleaseModioOtherUrlList(CModioOtherUrlList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioOtherUrlList* CopyModioOtherUrlList(const CModioOtherUrlList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioOtherUrlList { *Item };
}


MODIODLL_EXPORT CModioOtherUrl* GetModioOtherUrlListOtherURLbyIndex(const CModioOtherUrlList* ModioOtherUrlList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioOtherUrlList).size())
	{
		return new CModioOtherUrl{ Modio::GetImpl(ModioOtherUrlList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioOtherUrlListOtherURLbyIndex(CModioOtherUrlList* ModioOtherUrlList, uint64_t Index, const CModioOtherUrl * Value)
{
	if (Index < Modio::GetImpl(ModioOtherUrlList).size())
	{
			Modio::GetImpl(ModioOtherUrlList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioOtherUrlListCount(const CModioOtherUrlList* ModioOtherUrlList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioOtherUrlList).size());
}

MODIODLL_EXPORT void SetModioOtherUrlListCount(CModioOtherUrlList* ModioOtherUrlList, uint64_t Count)
{
	Modio::GetImpl(ModioOtherUrlList).resize(Count);
}


MODIODLL_EXPORT CModioTheme* CreateModioTheme()
{
	return new CModioTheme{ Modio::Theme {} };
}

MODIODLL_EXPORT void ReleaseModioTheme(CModioTheme* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioTheme* CopyModioTheme(const CModioTheme* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioTheme { *Item };
}

MODIODLL_EXPORT CModioString* GetModioThemePrimary(const CModioTheme* Theme)
{
	return new CModioString{ Theme->Impl.Primary };
}

MODIODLL_EXPORT CModioString* GetModioThemeDark(const CModioTheme* Theme)
{
	return new CModioString{ Theme->Impl.Dark };
}

MODIODLL_EXPORT CModioString* GetModioThemeLight(const CModioTheme* Theme)
{
	return new CModioString{ Theme->Impl.Light };
}

MODIODLL_EXPORT CModioString* GetModioThemeSuccess(const CModioTheme* Theme)
{
	return new CModioString{ Theme->Impl.Success };
}

MODIODLL_EXPORT CModioString* GetModioThemeWarning(const CModioTheme* Theme)
{
	return new CModioString{ Theme->Impl.Warning };
}

MODIODLL_EXPORT CModioString* GetModioThemeDanger(const CModioTheme* Theme)
{
	return new CModioString{ Theme->Impl.Danger };
}


MODIODLL_EXPORT void SetModioThemePrimary(CModioTheme* Item, const CModioString* Primary)
{
	if (Item != nullptr)
	{
		Item->Impl.Primary = Modio::GetImpl(Primary);
	}
}
MODIODLL_EXPORT void SetModioThemeDark(CModioTheme* Item, const CModioString* Dark)
{
	if (Item != nullptr)
	{
		Item->Impl.Dark = Modio::GetImpl(Dark);
	}
}
MODIODLL_EXPORT void SetModioThemeLight(CModioTheme* Item, const CModioString* Light)
{
	if (Item != nullptr)
	{
		Item->Impl.Light = Modio::GetImpl(Light);
	}
}
MODIODLL_EXPORT void SetModioThemeSuccess(CModioTheme* Item, const CModioString* Success)
{
	if (Item != nullptr)
	{
		Item->Impl.Success = Modio::GetImpl(Success);
	}
}
MODIODLL_EXPORT void SetModioThemeWarning(CModioTheme* Item, const CModioString* Warning)
{
	if (Item != nullptr)
	{
		Item->Impl.Warning = Modio::GetImpl(Warning);
	}
}
MODIODLL_EXPORT void SetModioThemeDanger(CModioTheme* Item, const CModioString* Danger)
{
	if (Item != nullptr)
	{
		Item->Impl.Danger = Modio::GetImpl(Danger);
	}
}

MODIODLL_EXPORT CModioGameStats* CreateModioGameStats()
{
	return new CModioGameStats{ Modio::GameStats {} };
}

MODIODLL_EXPORT void ReleaseModioGameStats(CModioGameStats* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioGameStats* CopyModioGameStats(const CModioGameStats* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioGameStats { *Item };
}

MODIODLL_EXPORT CModioGameID* GetModioGameStatsGameID(const CModioGameStats* GameStats)
{
	return new CModioGameID{ GameStats->Impl.GameID };
}

MODIODLL_EXPORT int64_t GetModioGameStatsModCountTotal(const CModioGameStats* GameStats)
{
	return GameStats->Impl.ModCountTotal;
}

MODIODLL_EXPORT int64_t GetModioGameStatsModDownloadsToday(const CModioGameStats* GameStats)
{
	return GameStats->Impl.ModDownloadsToday;
}

MODIODLL_EXPORT int64_t GetModioGameStatsModDownloadsTotal(const CModioGameStats* GameStats)
{
	return GameStats->Impl.ModDownloadsTotal;
}

MODIODLL_EXPORT int64_t GetModioGameStatsModDownloadsDailyAverage(const CModioGameStats* GameStats)
{
	return GameStats->Impl.ModDownloadsDailyAverage;
}

MODIODLL_EXPORT int64_t GetModioGameStatsModSubscribersTotal(const CModioGameStats* GameStats)
{
	return GameStats->Impl.ModSubscribersTotal;
}

MODIODLL_EXPORT int64_t GetModioGameStatsDateExpires(const CModioGameStats* GameStats)
{
	return GameStats->Impl.DateExpires;
}


MODIODLL_EXPORT void SetModioGameStatsGameID(CModioGameStats* Item, const CModioGameID* GameID)
{
	if (Item != nullptr)
	{
		Item->Impl.GameID = Modio::GetImpl(GameID);
	}
}
MODIODLL_EXPORT void SetModioGameStatsModCountTotal(CModioGameStats* Item, const int64_t ModCountTotal)
{
	if (Item != nullptr)
	{
		Item->Impl.ModCountTotal = ModCountTotal;
	}
}
MODIODLL_EXPORT void SetModioGameStatsModDownloadsToday(CModioGameStats* Item, const int64_t ModDownloadsToday)
{
	if (Item != nullptr)
	{
		Item->Impl.ModDownloadsToday = ModDownloadsToday;
	}
}
MODIODLL_EXPORT void SetModioGameStatsModDownloadsTotal(CModioGameStats* Item, const int64_t ModDownloadsTotal)
{
	if (Item != nullptr)
	{
		Item->Impl.ModDownloadsTotal = ModDownloadsTotal;
	}
}
MODIODLL_EXPORT void SetModioGameStatsModDownloadsDailyAverage(CModioGameStats* Item, const int64_t ModDownloadsDailyAverage)
{
	if (Item != nullptr)
	{
		Item->Impl.ModDownloadsDailyAverage = ModDownloadsDailyAverage;
	}
}
MODIODLL_EXPORT void SetModioGameStatsModSubscribersTotal(CModioGameStats* Item, const int64_t ModSubscribersTotal)
{
	if (Item != nullptr)
	{
		Item->Impl.ModSubscribersTotal = ModSubscribersTotal;
	}
}
MODIODLL_EXPORT void SetModioGameStatsDateExpires(CModioGameStats* Item, const int64_t DateExpires)
{
	if (Item != nullptr)
	{
		Item->Impl.DateExpires = DateExpires;
	}
}

MODIODLL_EXPORT CModioTermsOfUse* CreateModioTermsOfUse()
{
	return new CModioTermsOfUse{ Modio::Terms {} };
}

MODIODLL_EXPORT void ReleaseModioTermsOfUse(CModioTermsOfUse* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioTermsOfUse* CopyModioTermsOfUse(const CModioTermsOfUse* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioTermsOfUse { *Item };
}

MODIODLL_EXPORT CModioLink* GetModioTermsOfUseWebsiteLink(const CModioTermsOfUse* ModioTermsOfUse)
{
	return new CModioLink{ ModioTermsOfUse->Impl.Links.Website };
}

MODIODLL_EXPORT CModioLink* GetModioTermsOfUseTermsLink(const CModioTermsOfUse* ModioTermsOfUse)
{
	return new CModioLink{ ModioTermsOfUse->Impl.Links.Terms };
}

MODIODLL_EXPORT CModioLink* GetModioTermsOfUsePrivacyLink(const CModioTermsOfUse* ModioTermsOfUse)
{
	return new CModioLink{ ModioTermsOfUse->Impl.Links.Privacy };
}

MODIODLL_EXPORT CModioLink* GetModioTermsOfUseManageLink(const CModioTermsOfUse* ModioTermsOfUse)
{
	return new CModioLink{ ModioTermsOfUse->Impl.Links.Manage };
}


MODIODLL_EXPORT void SetModioTermsOfUseWebsiteLink(CModioTermsOfUse* Item, const CModioLink* WebsiteLink)
{
	if (Item != nullptr)
	{
		Item->Impl.Links.Website = Modio::GetImpl(WebsiteLink);
	}
}
MODIODLL_EXPORT void SetModioTermsOfUseTermsLink(CModioTermsOfUse* Item, const CModioLink* TermsLink)
{
	if (Item != nullptr)
	{
		Item->Impl.Links.Terms = Modio::GetImpl(TermsLink);
	}
}
MODIODLL_EXPORT void SetModioTermsOfUsePrivacyLink(CModioTermsOfUse* Item, const CModioLink* PrivacyLink)
{
	if (Item != nullptr)
	{
		Item->Impl.Links.Privacy = Modio::GetImpl(PrivacyLink);
	}
}
MODIODLL_EXPORT void SetModioTermsOfUseManageLink(CModioTermsOfUse* Item, const CModioLink* ManageLink)
{
	if (Item != nullptr)
	{
		Item->Impl.Links.Manage = Modio::GetImpl(ManageLink);
	}
}

MODIODLL_EXPORT CModioModTagList* CreateModioModTagList()
{
	return new CModioModTagList{ std::vector<Modio::ModTag> {} };
}

MODIODLL_EXPORT void ReleaseModioModTagList(CModioModTagList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModTagList* CopyModioModTagList(const CModioModTagList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModTagList { *Item };
}


MODIODLL_EXPORT CModioTag* GetModioModTagListTagByIndex(const CModioModTagList* ModioModTagList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioModTagList).size())
	{
		return new CModioTag{ Modio::GetImpl(ModioModTagList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioModTagListTagByIndex(CModioModTagList* ModioModTagList, uint64_t Index, const CModioTag * Value)
{
	if (Index < Modio::GetImpl(ModioModTagList).size())
	{
			Modio::GetImpl(ModioModTagList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioModTagListCount(const CModioModTagList* ModioModTagList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioModTagList).size());
}

MODIODLL_EXPORT void SetModioModTagListCount(CModioModTagList* ModioModTagList, uint64_t Count)
{
	Modio::GetImpl(ModioModTagList).resize(Count);
}


MODIODLL_EXPORT CModioModStats* CreateModioModStats()
{
	return new CModioModStats{ Modio::ModStats {} };
}

MODIODLL_EXPORT void ReleaseModioModStats(CModioModStats* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModStats* CopyModioModStats(const CModioModStats* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModStats { *Item };
}

MODIODLL_EXPORT int64_t GetModioModStatsPopularityRankPosition(const CModioModStats* ModioModStats)
{
	return int64_t{ ModioModStats->Impl.PopularityRankPosition };
}

MODIODLL_EXPORT int64_t GetModioModStatsPopularityRankTotalMods(const CModioModStats* ModioModStats)
{
	return int64_t{ ModioModStats->Impl.PopularityRankTotalMods };
}

MODIODLL_EXPORT int64_t GetModioModStatsDownloadsTotal(const CModioModStats* ModioModStats)
{
	return int64_t{ ModioModStats->Impl.DownloadsTotal };
}

MODIODLL_EXPORT int64_t GetModioModStatsSubscribersTotal(const CModioModStats* ModioModStats)
{
	return int64_t{ ModioModStats->Impl.SubscribersTotal };
}

MODIODLL_EXPORT int64_t GetModioModStatsRatingTotal(const CModioModStats* ModioModStats)
{
	return int64_t{ ModioModStats->Impl.RatingTotal };
}

MODIODLL_EXPORT int64_t GetModioModStatsRatingPositive(const CModioModStats* ModioModStats)
{
	return int64_t{ ModioModStats->Impl.RatingPositive };
}

MODIODLL_EXPORT int64_t GetModioModStatsRatingNegative(const CModioModStats* ModioModStats)
{
	return int64_t{ ModioModStats->Impl.RatingNegative };
}

MODIODLL_EXPORT int64_t GetModioModStatsRatingPercentagePositive(const CModioModStats* ModioModStats)
{
	return int64_t{ ModioModStats->Impl.RatingPercentagePositive };
}

MODIODLL_EXPORT double GetModioModStatsRatingWeightedAggregate(const CModioModStats* ModioModStats)
{
	return double{ ModioModStats->Impl.RatingWeightedAggregate };
}

MODIODLL_EXPORT CModioString* GetModioModStatsRatingDisplayText(const CModioModStats* ModioModStats)
{
	return new CModioString{ ModioModStats->Impl.RatingDisplayText };
}


MODIODLL_EXPORT void SetModioModStatsPopularityRankPosition(CModioModStats* Item, const int64_t PopularityRankPosition)
{
	if (Item != nullptr)
	{
		Item->Impl.PopularityRankPosition = PopularityRankPosition;
	}
}
MODIODLL_EXPORT void SetModioModStatsPopularityRankTotalMods(CModioModStats* Item, const int64_t PopularityRankTotalMods)
{
	if (Item != nullptr)
	{
		Item->Impl.PopularityRankTotalMods = PopularityRankTotalMods;
	}
}
MODIODLL_EXPORT void SetModioModStatsDownloadsTotal(CModioModStats* Item, const int64_t DownloadsTotal)
{
	if (Item != nullptr)
	{
		Item->Impl.DownloadsTotal = DownloadsTotal;
	}
}
MODIODLL_EXPORT void SetModioModStatsSubscribersTotal(CModioModStats* Item, const int64_t SubscribersTotal)
{
	if (Item != nullptr)
	{
		Item->Impl.SubscribersTotal = SubscribersTotal;
	}
}
MODIODLL_EXPORT void SetModioModStatsRatingTotal(CModioModStats* Item, const int64_t RatingTotal)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingTotal = RatingTotal;
	}
}
MODIODLL_EXPORT void SetModioModStatsRatingPositive(CModioModStats* Item, const int64_t RatingPositive)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingPositive = RatingPositive;
	}
}
MODIODLL_EXPORT void SetModioModStatsRatingNegative(CModioModStats* Item, const int64_t RatingNegative)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingNegative = RatingNegative;
	}
}
MODIODLL_EXPORT void SetModioModStatsRatingPercentagePositive(CModioModStats* Item, const int64_t RatingPercentagePositive)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingPercentagePositive = RatingPercentagePositive;
	}
}
MODIODLL_EXPORT void SetModioModStatsRatingWeightedAggregate(CModioModStats* Item, const double RatingWeightedAggregate)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingWeightedAggregate = RatingWeightedAggregate;
	}
}
MODIODLL_EXPORT void SetModioModStatsRatingDisplayText(CModioModStats* Item, const CModioString* RatingDisplayText)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingDisplayText = Modio::GetImpl(RatingDisplayText);
	}
}

MODIODLL_EXPORT CModioModCollectionStats* CreateModioModCollectionStats()
{
	return new CModioModCollectionStats{ Modio::ModCollectionStats {} };
}

MODIODLL_EXPORT void ReleaseModioModCollectionStats(CModioModCollectionStats* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModCollectionStats* CopyModioModCollectionStats(const CModioModCollectionStats* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModCollectionStats { *Item };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsDownloadsToday(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.DownloadsToday };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsDownloadsTotal(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.DownloadsTotal };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsDownloadsUnique(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.DownloadsUnique };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsRatingTotal30Days(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.RatingTotal30Days };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsRatingPositive30Days(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.RatingPositive30Days };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsRatingNegative30Days(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.RatingNegative30Days };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsRatingTotal(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.RatingTotal };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsRatingPositive(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.RatingPositive };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsRatingNegative(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.RatingNegative };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsNumberOfComments(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.NumberOfComments };
}

MODIODLL_EXPORT int64_t GetModioModCollectionStatsNumberOfMods(const CModioModCollectionStats* ModCollectionStats)
{
	return int64_t{ ModCollectionStats->Impl.NumberOfMods };
}


MODIODLL_EXPORT void SetModioModCollectionStatsDownloadsToday(CModioModCollectionStats* Item, const int64_t DownloadsToday)
{
	if (Item != nullptr)
	{
		Item->Impl.DownloadsToday = DownloadsToday;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsDownloadsTotal(CModioModCollectionStats* Item, const int64_t DownloadsTotal)
{
	if (Item != nullptr)
	{
		Item->Impl.DownloadsTotal = DownloadsTotal;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsDownloadsUnique(CModioModCollectionStats* Item, const int64_t DownloadsUnique)
{
	if (Item != nullptr)
	{
		Item->Impl.DownloadsUnique = DownloadsUnique;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsRatingTotal30Days(CModioModCollectionStats* Item, const int64_t RatingTotal30Days)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingTotal30Days = RatingTotal30Days;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsRatingPositive30Days(CModioModCollectionStats* Item, const int64_t RatingPositive30Days)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingPositive30Days = RatingPositive30Days;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsRatingNegative30Days(CModioModCollectionStats* Item, const int64_t RatingNegative30Days)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingNegative30Days = RatingNegative30Days;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsRatingTotal(CModioModCollectionStats* Item, const int64_t RatingTotal)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingTotal = RatingTotal;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsRatingPositive(CModioModCollectionStats* Item, const int64_t RatingPositive)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingPositive = RatingPositive;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsRatingNegative(CModioModCollectionStats* Item, const int64_t RatingNegative)
{
	if (Item != nullptr)
	{
		Item->Impl.RatingNegative = RatingNegative;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsNumberOfComments(CModioModCollectionStats* Item, const int64_t NumberOfComments)
{
	if (Item != nullptr)
	{
		Item->Impl.NumberOfComments = NumberOfComments;
	}
}
MODIODLL_EXPORT void SetModioModCollectionStatsNumberOfMods(CModioModCollectionStats* Item, const int64_t NumberOfMods)
{
	if (Item != nullptr)
	{
		Item->Impl.NumberOfMods = NumberOfMods;
	}
}

MODIODLL_EXPORT CModioGameInfo* CreateModioGameInfo()
{
	return new CModioGameInfo{ Modio::GameInfo {} };
}

MODIODLL_EXPORT void ReleaseModioGameInfo(CModioGameInfo* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioGameInfo* CopyModioGameInfo(const CModioGameInfo* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioGameInfo { *Item };
}

MODIODLL_EXPORT CModioGameID* GetModioGameInfoGameID(const CModioGameInfo* ModioGameInfo)
{
	return new CModioGameID{ ModioGameInfo->Impl.GameID };
}

MODIODLL_EXPORT int64_t GetModioGameInfoDateAdded(const CModioGameInfo* ModioGameInfo)
{
	return int64_t{ ModioGameInfo->Impl.DateAdded };
}

MODIODLL_EXPORT int64_t GetModioGameInfoDateUpdated(const CModioGameInfo* ModioGameInfo)
{
	return int64_t{ ModioGameInfo->Impl.DateUpdated };
}

MODIODLL_EXPORT int64_t GetModioGameInfoDateLive(const CModioGameInfo* ModioGameInfo)
{
	return int64_t{ ModioGameInfo->Impl.DateLive };
}

MODIODLL_EXPORT CModioString* GetModioGameInfoUgcName(const CModioGameInfo* ModioGameInfo)
{
	return new CModioString{ ModioGameInfo->Impl.UgcName };
}

MODIODLL_EXPORT CModioIcon* GetModioGameInfoIcon(const CModioGameInfo* ModioGameInfo)
{
	return new CModioIcon{ ModioGameInfo->Impl.Icon };
}

MODIODLL_EXPORT CModioLogo* GetModioGameInfoLogo(const CModioGameInfo* ModioGameInfo)
{
	return new CModioLogo{ ModioGameInfo->Impl.Logo };
}

MODIODLL_EXPORT CModioHeaderImage* GetModioGameInfoHeaderImage(const CModioGameInfo* ModioGameInfo)
{
	return new CModioHeaderImage{ ModioGameInfo->Impl.HeaderImage };
}

MODIODLL_EXPORT CModioString* GetModioGameInfoName(const CModioGameInfo* ModioGameInfo)
{
	return new CModioString{ ModioGameInfo->Impl.Name };
}

MODIODLL_EXPORT CModioString* GetModioGameInfoSummary(const CModioGameInfo* ModioGameInfo)
{
	return new CModioString{ ModioGameInfo->Impl.Summary };
}

MODIODLL_EXPORT CModioString* GetModioGameInfoInstructions(const CModioGameInfo* ModioGameInfo)
{
	return new CModioString{ ModioGameInfo->Impl.Instructions };
}

MODIODLL_EXPORT CModioString* GetModioGameInfoInstructionsUrl(const CModioGameInfo* ModioGameInfo)
{
	return new CModioString{ ModioGameInfo->Impl.InstructionsUrl };
}

MODIODLL_EXPORT CModioString* GetModioGameInfoProfileUrl(const CModioGameInfo* ModioGameInfo)
{
	return new CModioString{ ModioGameInfo->Impl.ProfileUrl };
}

MODIODLL_EXPORT CModioTheme* GetModioGameInfoTheme(const CModioGameInfo* ModioGameInfo)
{
	return new CModioTheme{ ModioGameInfo->Impl.Theme };
}

MODIODLL_EXPORT CModioGameStats* GetModioGameInfoStats(const CModioGameInfo* ModioGameInfo)
{
	return new CModioGameStats{ ModioGameInfo->Impl.Stats };
}

MODIODLL_EXPORT CModioOtherUrlList* GetModioGameInfoOtherUrls(const CModioGameInfo* ModioGameInfo)
{
	return new CModioOtherUrlList{ ModioGameInfo->Impl.OtherUrls };
}

MODIODLL_EXPORT CModioGamePlatformList* GetModioGameInfoPlatformSupport(const CModioGameInfo* ModioGameInfo)
{
	return new CModioGamePlatformList{ ModioGameInfo->Impl.PlatformSupport };
}

MODIODLL_EXPORT CModioGameCommunityOptionsFlags GetModioGameInfoCommunityOptions(const CModioGameInfo* ModioGameInfo)
{
	return CModioGameCommunityOptionsFlags(ModioGameInfo->Impl.CommunityOptions.RawValue());
}

MODIODLL_EXPORT CModioMaturityProfile GetModioGameInfoMaturityOptions(const CModioGameInfo* ModInfo)
{
	return CModioMaturityProfile(ModInfo->Impl.MaturityOptions.RawValue());
}

MODIODLL_EXPORT CModioString* GetModioGameInfoVirtualTokenName(const CModioGameInfo* ModioGameInfo)
{
	return new CModioString{ ModioGameInfo->Impl.VirtualTokenName };
}

MODIODLL_EXPORT CModioModTagInfoList* GetModioGameInfoTagOptions(const CModioGameInfo* ModioGameInfo)
{
	return new CModioModTagInfoList{ ModioGameInfo->Impl.TagOptions };
}


MODIODLL_EXPORT void SetModioGameInfoGameID(CModioGameInfo* Item, const CModioGameID* GameID)
{
	if (Item != nullptr)
	{
		Item->Impl.GameID = Modio::GetImpl(GameID);
	}
}
MODIODLL_EXPORT void SetModioGameInfoDateAdded(CModioGameInfo* Item, const int64_t DateAdded)
{
	if (Item != nullptr)
	{
		Item->Impl.DateAdded = DateAdded;
	}
}
MODIODLL_EXPORT void SetModioGameInfoDateUpdated(CModioGameInfo* Item, const int64_t DateUpdated)
{
	if (Item != nullptr)
	{
		Item->Impl.DateUpdated = DateUpdated;
	}
}
MODIODLL_EXPORT void SetModioGameInfoDateLive(CModioGameInfo* Item, const int64_t DateLive)
{
	if (Item != nullptr)
	{
		Item->Impl.DateLive = DateLive;
	}
}
MODIODLL_EXPORT void SetModioGameInfoUgcName(CModioGameInfo* Item, const CModioString* UgcName)
{
	if (Item != nullptr)
	{
		Item->Impl.UgcName = Modio::GetImpl(UgcName);
	}
}
MODIODLL_EXPORT void SetModioGameInfoIcon(CModioGameInfo* Item, const CModioIcon* Icon)
{
	if (Item != nullptr)
	{
		Item->Impl.Icon = Modio::GetImpl(Icon);
	}
}
MODIODLL_EXPORT void SetModioGameInfoLogo(CModioGameInfo* Item, const CModioLogo* Logo)
{
	if (Item != nullptr)
	{
		Item->Impl.Logo = Modio::GetImpl(Logo);
	}
}
MODIODLL_EXPORT void SetModioGameInfoHeaderImage(CModioGameInfo* Item, const CModioHeaderImage* HeaderImage)
{
	if (Item != nullptr)
	{
		Item->Impl.HeaderImage = Modio::GetImpl(HeaderImage);
	}
}
MODIODLL_EXPORT void SetModioGameInfoName(CModioGameInfo* Item, const CModioString* Name)
{
	if (Item != nullptr)
	{
		Item->Impl.Name = Modio::GetImpl(Name);
	}
}
MODIODLL_EXPORT void SetModioGameInfoSummary(CModioGameInfo* Item, const CModioString* Summary)
{
	if (Item != nullptr)
	{
		Item->Impl.Summary = Modio::GetImpl(Summary);
	}
}
MODIODLL_EXPORT void SetModioGameInfoInstructions(CModioGameInfo* Item, const CModioString* Instructions)
{
	if (Item != nullptr)
	{
		Item->Impl.Instructions = Modio::GetImpl(Instructions);
	}
}
MODIODLL_EXPORT void SetModioGameInfoInstructionsUrl(CModioGameInfo* Item, const CModioString* InstructionsUrl)
{
	if (Item != nullptr)
	{
		Item->Impl.InstructionsUrl = Modio::GetImpl(InstructionsUrl);
	}
}
MODIODLL_EXPORT void SetModioGameInfoProfileUrl(CModioGameInfo* Item, const CModioString* ProfileUrl)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileUrl = Modio::GetImpl(ProfileUrl);
	}
}
MODIODLL_EXPORT void SetModioGameInfoTheme(CModioGameInfo* Item, const CModioTheme* Theme)
{
	if (Item != nullptr)
	{
		Item->Impl.Theme = Modio::GetImpl(Theme);
	}
}
MODIODLL_EXPORT void SetModioGameInfoStats(CModioGameInfo* Item, const CModioGameStats* Stats)
{
	if (Item != nullptr)
	{
		Item->Impl.Stats = Modio::GetImpl(Stats);
	}
}
MODIODLL_EXPORT void SetModioGameInfoOtherUrls(CModioGameInfo* Item, const CModioOtherUrlList* OtherUrls)
{
	if (Item != nullptr)
	{
		Item->Impl.OtherUrls = Modio::GetImpl(OtherUrls);
	}
}
MODIODLL_EXPORT void SetModioGameInfoPlatformSupport(CModioGameInfo* Item, const CModioGamePlatformList* PlatformSupport)
{
	if (Item != nullptr)
	{
		Item->Impl.PlatformSupport = Modio::GetImpl(PlatformSupport);
	}
}
MODIODLL_EXPORT void SetModioGameInfoCommunityOptions(CModioGameInfo* Item, CModioGameCommunityOptionsFlags CommunityOptions)
{
	if (Item != nullptr)
	{
		Item->Impl.CommunityOptions = static_cast<Modio::ProfileMaturity::StorageType>(CommunityOptions);
	}
}
MODIODLL_EXPORT void SetModioGameInfoMaturityOptions(CModioGameInfo* Item, CModioMaturityProfile MaturityOptions)
{
	if (Item != nullptr)
	{
		Item->Impl.MaturityOptions = static_cast<Modio::GameMaturityOptionsFlags::StorageType>(MaturityOptions);
	}
}
MODIODLL_EXPORT void SetModioGameInfoVirtualTokenName(CModioGameInfo* Item, const CModioString* VirtualTokenName)
{
	if (Item != nullptr)
	{
		Item->Impl.VirtualTokenName = Modio::GetImpl(VirtualTokenName);
	}
}
MODIODLL_EXPORT void SetModioGameInfoTagOptions(CModioGameInfo* Item, const CModioModTagInfoList* TagOptions)
{
	if (Item != nullptr)
	{
		Item->Impl.TagOptions = Modio::GetImpl(TagOptions);
	}
}

MODIODLL_EXPORT CModioModLogo* CreateModioModLogo()
{
	return new CModioModLogo{ Modio::Detail::Logo {} };
}

MODIODLL_EXPORT void ReleaseModioModLogo(CModioModLogo* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModLogo* CopyModioModLogo(const CModioModLogo* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModLogo { *Item };
}

MODIODLL_EXPORT CModioString* GetModioModLogoFilename(const CModioModLogo* ModioModLogo)
{
	return new CModioString{ ModioModLogo->Impl.Filename };
}

MODIODLL_EXPORT CModioString* GetModioModLogoOriginal(const CModioModLogo* ModioModLogo)
{
	return new CModioString{ ModioModLogo->Impl.Original };
}

MODIODLL_EXPORT CModioString* GetModioModLogoThumb320x180(const CModioModLogo* ModioModLogo)
{
	return new CModioString{ ModioModLogo->Impl.Thumb320x180 };
}

MODIODLL_EXPORT CModioString* GetModioModLogoThumb640x360(const CModioModLogo* ModioModLogo)
{
	return new CModioString{ ModioModLogo->Impl.Thumb640x360 };
}

MODIODLL_EXPORT CModioString* GetModioModLogoThumb1280x720(const CModioModLogo* ModioModLogo)
{
	return new CModioString{ ModioModLogo->Impl.Thumb1280x720 };
}


MODIODLL_EXPORT void SetModioModLogoFilename(CModioModLogo* Item, const CModioString* Filename)
{
	if (Item != nullptr)
	{
		Item->Impl.Filename = Modio::GetImpl(Filename);
	}
}
MODIODLL_EXPORT void SetModioModLogoOriginal(CModioModLogo* Item, const CModioString* Original)
{
	if (Item != nullptr)
	{
		Item->Impl.Original = Modio::GetImpl(Original);
	}
}
MODIODLL_EXPORT void SetModioModLogoThumb320x180(CModioModLogo* Item, const CModioString* Thumb320x180)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb320x180 = Modio::GetImpl(Thumb320x180);
	}
}
MODIODLL_EXPORT void SetModioModLogoThumb640x360(CModioModLogo* Item, const CModioString* Thumb640x360)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb640x360 = Modio::GetImpl(Thumb640x360);
	}
}
MODIODLL_EXPORT void SetModioModLogoThumb1280x720(CModioModLogo* Item, const CModioString* Thumb1280x720)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb1280x720 = Modio::GetImpl(Thumb1280x720);
	}
}

MODIODLL_EXPORT CModioKVP* CreateModioKVP()
{
	return new CModioKVP{ Modio::Metadata {} };
}

MODIODLL_EXPORT void ReleaseModioKVP(CModioKVP* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioKVP* CopyModioKVP(const CModioKVP* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioKVP { *Item };
}

MODIODLL_EXPORT CModioString* GetModioKVPKey(const CModioKVP* ModioKVP)
{
	return new CModioString{ ModioKVP->Impl.Key };
}

MODIODLL_EXPORT CModioString* GetModioKVPValue(const CModioKVP* ModioKVP)
{
	return new CModioString{ ModioKVP->Impl.Value };
}


MODIODLL_EXPORT void SetModioKVPKey(CModioKVP* Item, const CModioString* Key)
{
	if (Item != nullptr)
	{
		Item->Impl.Key = Modio::GetImpl(Key);
	}
}
MODIODLL_EXPORT void SetModioKVPValue(CModioKVP* Item, const CModioString* Value)
{
	if (Item != nullptr)
	{
		Item->Impl.Value = Modio::GetImpl(Value);
	}
}

MODIODLL_EXPORT CModioMetadataKVP* CreateModioMetadataKVP()
{
	return new CModioMetadataKVP{ std::vector<Modio::Metadata> {} };
}

MODIODLL_EXPORT void ReleaseModioMetadataKVP(CModioMetadataKVP* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioMetadataKVP* CopyModioMetadataKVP(const CModioMetadataKVP* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioMetadataKVP { *Item };
}


MODIODLL_EXPORT CModioKVP* GetModioMetadataKVPKVPByIndex(const CModioMetadataKVP* ModioMetadataKVP, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioMetadataKVP).size())
	{
		return new CModioKVP{ Modio::GetImpl(ModioMetadataKVP)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioMetadataKVPKVPByIndex(CModioMetadataKVP* ModioMetadataKVP, uint64_t Index, const CModioKVP * Value)
{
	if (Index < Modio::GetImpl(ModioMetadataKVP).size())
	{
			Modio::GetImpl(ModioMetadataKVP)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioMetadataKVPCount(const CModioMetadataKVP* ModioMetadataKVP)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioMetadataKVP).size());
}

MODIODLL_EXPORT void SetModioMetadataKVPCount(CModioMetadataKVP* ModioMetadataKVP, uint64_t Count)
{
	Modio::GetImpl(ModioMetadataKVP).resize(Count);
}


MODIODLL_EXPORT CModioFileInfo* CreateModioFileInfo()
{
	return new CModioFileInfo{ Modio::FileMetadata {} };
}

MODIODLL_EXPORT void ReleaseModioFileInfo(CModioFileInfo* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioFileInfo* CopyModioFileInfo(const CModioFileInfo* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioFileInfo { *Item };
}

MODIODLL_EXPORT CModioFileMetadataID* GetModioFileInfoMetadataID(const CModioFileInfo* ModioFileInfo)
{
	return new CModioFileMetadataID{ ModioFileInfo->Impl.MetadataId };
}

MODIODLL_EXPORT CModioModID* GetModioFileInfoModID(const CModioFileInfo* ModioFileInfo)
{
	return new CModioModID{ ModioFileInfo->Impl.ModId };
}

MODIODLL_EXPORT int64_t GetModioFileInfoDateAdded(const CModioFileInfo* ModioFileInfo)
{
	return int64_t{ ModioFileInfo->Impl.DateAdded };
}

MODIODLL_EXPORT EModioVirusScanStatus GetModioFileInfoVirusScanStatus(const CModioFileInfo* ModioFileInfo)
{
	return ConvertEnumToC( ModioFileInfo->Impl.CurrentVirusScanStatus );
}

MODIODLL_EXPORT EModioVirusStatus GetModioFileInfoVirusStatus(const CModioFileInfo* ModioFileInfo)
{
	return ConvertEnumToC( ModioFileInfo->Impl.CurrentVirusStatus );
}

MODIODLL_EXPORT uint64_t GetModioFileInfoFileSize(const CModioFileInfo* ModioFileInfo)
{
	return uint64_t{ ModioFileInfo->Impl.Filesize };
}

MODIODLL_EXPORT uint64_t GetModioFileInfoFileSizeUncompressed(const CModioFileInfo* ModioFileInfo)
{
	return uint64_t{ ModioFileInfo->Impl.FilesizeUncompressed };
}

MODIODLL_EXPORT CModioString* GetModioFileInfoFilename(const CModioFileInfo* ModioFileInfo)
{
	return new CModioString{ ModioFileInfo->Impl.Filename };
}

MODIODLL_EXPORT CModioString* GetModioFileInfoVersion(const CModioFileInfo* ModioFileInfo)
{
	return new CModioString{ ModioFileInfo->Impl.Version };
}

MODIODLL_EXPORT CModioString* GetModioFileInfoChangelog(const CModioFileInfo* ModioFileInfo)
{
	return new CModioString{ ModioFileInfo->Impl.Changelog };
}

MODIODLL_EXPORT CModioString* GetModioFileInfoMetadataBlob(const CModioFileInfo* ModioFileInfo)
{
	return new CModioString{ ModioFileInfo->Impl.MetadataBlob };
}

MODIODLL_EXPORT CModioString* GetModioFileInfoDownloadBinaryURL(const CModioFileInfo* ModioFileInfo)
{
	return new CModioString{ ModioFileInfo->Impl.DownloadBinaryURL };
}

MODIODLL_EXPORT int64_t GetModioFileInfoDownloadExpiryDate(const CModioFileInfo* ModioFileInfo)
{
	return int64_t{ ModioFileInfo->Impl.DownloadExpiryDate };
}


MODIODLL_EXPORT void SetModioFileInfoMetadataID(CModioFileInfo* Item, const CModioFileMetadataID* MetadataID)
{
	if (Item != nullptr)
	{
		Item->Impl.MetadataId = Modio::GetImpl(MetadataID);
	}
}
MODIODLL_EXPORT void SetModioFileInfoModID(CModioFileInfo* Item, const CModioModID* ModID)
{
	if (Item != nullptr)
	{
		Item->Impl.ModId = Modio::GetImpl(ModID);
	}
}
MODIODLL_EXPORT void SetModioFileInfoDateAdded(CModioFileInfo* Item, const int64_t DateAdded)
{
	if (Item != nullptr)
	{
		Item->Impl.DateAdded = DateAdded;
	}
}
MODIODLL_EXPORT void SetModioFileInfoVirusScanStatus(CModioFileInfo* Item, const EModioVirusScanStatus VirusScanStatus)
{
	if (Item != nullptr)
	{
		Item->Impl.CurrentVirusScanStatus = static_cast<Modio::FileMetadata::VirusScanStatus>(VirusScanStatus);
	}
}
MODIODLL_EXPORT void SetModioFileInfoVirusStatus(CModioFileInfo* Item, const EModioVirusStatus VirusStatus)
{
	if (Item != nullptr)
	{
		Item->Impl.CurrentVirusStatus = static_cast<Modio::FileMetadata::VirusStatus>(VirusStatus);
	}
}
MODIODLL_EXPORT void SetModioFileInfoFileSize(CModioFileInfo* Item, const uint64_t FileSize)
{
	if (Item != nullptr)
	{
		Item->Impl.Filesize = FileSize;
	}
}
MODIODLL_EXPORT void SetModioFileInfoFileSizeUncompressed(CModioFileInfo* Item, const uint64_t FileSizeUncompressed)
{
	if (Item != nullptr)
	{
		Item->Impl.FilesizeUncompressed = FileSizeUncompressed;
	}
}
MODIODLL_EXPORT void SetModioFileInfoFilename(CModioFileInfo* Item, const CModioString* Filename)
{
	if (Item != nullptr)
	{
		Item->Impl.Filename = Modio::GetImpl(Filename);
	}
}
MODIODLL_EXPORT void SetModioFileInfoVersion(CModioFileInfo* Item, const CModioString* Version)
{
	if (Item != nullptr)
	{
		Item->Impl.Version = Modio::GetImpl(Version);
	}
}
MODIODLL_EXPORT void SetModioFileInfoChangelog(CModioFileInfo* Item, const CModioString* Changelog)
{
	if (Item != nullptr)
	{
		Item->Impl.Changelog = Modio::GetImpl(Changelog);
	}
}
MODIODLL_EXPORT void SetModioFileInfoMetadataBlob(CModioFileInfo* Item, const CModioString* MetadataBlob)
{
	if (Item != nullptr)
	{
		Item->Impl.MetadataBlob = Modio::GetImpl(MetadataBlob);
	}
}
MODIODLL_EXPORT void SetModioFileInfoDownloadBinaryURL(CModioFileInfo* Item, const CModioString* DownloadBinaryURL)
{
	if (Item != nullptr)
	{
		Item->Impl.DownloadBinaryURL = Modio::GetImpl(DownloadBinaryURL);
	}
}
MODIODLL_EXPORT void SetModioFileInfoDownloadExpiryDate(CModioFileInfo* Item, const int64_t DownloadExpiryDate)
{
	if (Item != nullptr)
	{
		Item->Impl.DownloadExpiryDate = DownloadExpiryDate;
	}
}

MODIODLL_EXPORT CModioUser* CreateModioUser()
{
	return new CModioUser{ Modio::User {} };
}

MODIODLL_EXPORT void ReleaseModioUser(CModioUser* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioUser* CopyModioUser(const CModioUser* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioUser { *Item };
}

MODIODLL_EXPORT CModioUserID* GetModioUserUserID(const CModioUser* ModioUser)
{
	return new CModioUserID{ ModioUser->Impl.UserId };
}

MODIODLL_EXPORT CModioString* GetModioUserUsername(const CModioUser* ModioUser)
{
	return new CModioString{ ModioUser->Impl.Username };
}

MODIODLL_EXPORT CModioAuthToken* GetModioUserAuthToken(const CModioUser* ModioUser)
{
	if (ModioUser->Impl.AuthToken.has_value())
	{
		return new CModioAuthToken{ *(ModioUser->Impl.AuthToken) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT int64_t GetModioUserDateOnline(const CModioUser* ModioUser)
{
	return int64_t{ ModioUser->Impl.DateOnline };
}

MODIODLL_EXPORT CModioString* GetModioUserProfileURL(const CModioUser* ModioUser)
{
	return new CModioString{ ModioUser->Impl.ProfileUrl };
}

MODIODLL_EXPORT CModioAvatar* GetModioUserAvatar(const CModioUser* ModioUser)
{
	return new CModioAvatar{ ModioUser->Impl.Avatar };
}

MODIODLL_EXPORT CModioString* GetModioUserDisplayNamePortal(const CModioUser* ModioUser)
{
	return new CModioString{ ModioUser->Impl.DisplayNamePortal };
}


MODIODLL_EXPORT void SetModioUserUserID(CModioUser* Item, const CModioUserID* UserID)
{
	if (Item != nullptr)
	{
		Item->Impl.UserId = Modio::GetImpl(UserID);
	}
}
MODIODLL_EXPORT void SetModioUserUsername(CModioUser* Item, const CModioString* Username)
{
	if (Item != nullptr)
	{
		Item->Impl.Username = Modio::GetImpl(Username);
	}
}
MODIODLL_EXPORT void SetModioUserAuthToken(CModioUser* Item, const CModioAuthToken* AuthToken)
{
	if (Item != nullptr)
	{
		if(AuthToken)
		{
			Item->Impl.AuthToken = Modio::GetImpl(AuthToken);
		}
		else if(Item->Impl.AuthToken.has_value())
		{
			Item->Impl.AuthToken.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioUserDateOnline(CModioUser* Item, const int64_t DateOnline)
{
	if (Item != nullptr)
	{
		Item->Impl.DateOnline = DateOnline;
	}
}
MODIODLL_EXPORT void SetModioUserProfileURL(CModioUser* Item, const CModioString* ProfileURL)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileUrl = Modio::GetImpl(ProfileURL);
	}
}
MODIODLL_EXPORT void SetModioUserAvatar(CModioUser* Item, const CModioAvatar* Avatar)
{
	if (Item != nullptr)
	{
		Item->Impl.Avatar = Modio::GetImpl(Avatar);
	}
}
MODIODLL_EXPORT void SetModioUserDisplayNamePortal(CModioUser* Item, const CModioString* DisplayNamePortal)
{
	if (Item != nullptr)
	{
		Item->Impl.DisplayNamePortal = Modio::GetImpl(DisplayNamePortal);
	}
}

MODIODLL_EXPORT CModioFilterParams* CreateModioFilterParams()
{
	return new CModioFilterParams{ Modio::FilterParams {} };
}

MODIODLL_EXPORT void ReleaseModioFilterParams(CModioFilterParams* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioFilterParams* CopyModioFilterParams(const CModioFilterParams* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioFilterParams { *Item };
}


MODIODLL_EXPORT void ModioFilterParamsSortBy(CModioFilterParams *Item, EModioSortFieldType ByField, EModioSortDirection ByDirection)
{
		Modio::GetImpl(Item).SortBy(static_cast<Modio::FilterParams::SortFieldType>(ByField), static_cast<Modio::FilterParams::SortDirection>(ByDirection));
}

MODIODLL_EXPORT void ModioFilterParamsNameContains(CModioFilterParams *Item, CModioStringList const* SearchString)
{
		Modio::GetImpl(Item).NameContains(Modio::GetImpl(SearchString));
}

MODIODLL_EXPORT void ModioFilterParamsMatchingAuthor(CModioFilterParams *Item, CModioUserID const* UserId)
{
		Modio::GetImpl(Item).MatchingAuthor(Modio::GetImpl(UserId));
}

MODIODLL_EXPORT void ModioFilterParamsMatchingAuthors(CModioFilterParams *Item, CModioUserIDList const* UserIds)
{
		Modio::GetImpl(Item).MatchingAuthors(Modio::GetImpl(UserIds));
}

MODIODLL_EXPORT void ModioFilterParamsMatchingIDs(CModioFilterParams *Item, CModioModIDList const* IDSet)
{
		Modio::GetImpl(Item).MatchingIDs(Modio::GetImpl(IDSet));
}

MODIODLL_EXPORT void ModioFilterParamsExcludingIDs(CModioFilterParams *Item, CModioModIDList const* IDSet)
{
		Modio::GetImpl(Item).ExcludingIDs(Modio::GetImpl(IDSet));
}

MODIODLL_EXPORT void ModioFilterParamsWithTags(CModioFilterParams *Item, CModioStringList const* UserId)
{
		Modio::GetImpl(Item).WithTags(Modio::GetImpl(UserId));
}

MODIODLL_EXPORT void ModioFilterParamsWithoutTags(CModioFilterParams *Item, CModioStringList const* UserId)
{
		Modio::GetImpl(Item).WithoutTags(Modio::GetImpl(UserId));
}

MODIODLL_EXPORT void ModioFilterParamsMetadataLike(CModioFilterParams *Item, CModioString const* SearchString)
{
		Modio::GetImpl(Item).MetadataLike(Modio::GetImpl(SearchString));
}

MODIODLL_EXPORT void ModioFilterParamsIndexedResults(CModioFilterParams *Item, size_t StartIndex, size_t ResultCount)
{
		Modio::GetImpl(Item).IndexedResults((StartIndex), (ResultCount));
}

MODIODLL_EXPORT void ModioFilterParamsPagedResults(CModioFilterParams *Item, size_t PageNumber, size_t PageSize)
{
		Modio::GetImpl(Item).PagedResults((PageNumber), (PageSize));
}

MODIODLL_EXPORT void ModioFilterParamsRevenueType(CModioFilterParams *Item, EModioRevenueFilterType ByRevenue)
{
		Modio::GetImpl(Item).RevenueType(static_cast<Modio::FilterParams::RevenueFilterType>(ByRevenue));
}

MODIODLL_EXPORT void ModioFilterParamsDisallowMatureContent(CModioFilterParams *Item)
{
		Modio::GetImpl(Item).DisallowMatureContent();
}


MODIODLL_EXPORT CModioAvatar* CreateModioAvatar()
{
	return new CModioAvatar{ Modio::Detail::Avatar {} };
}

MODIODLL_EXPORT void ReleaseModioAvatar(CModioAvatar* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioAvatar* CopyModioAvatar(const CModioAvatar* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioAvatar { *Item };
}

MODIODLL_EXPORT CModioString* GetModioAvatarFilename(const CModioAvatar* ModioAvatar)
{
	return new CModioString{ ModioAvatar->Impl.Filename };
}

MODIODLL_EXPORT CModioString* GetModioAvatarFullSizeURL(const CModioAvatar* ModioAvatar)
{
	return new CModioString{ ModioAvatar->Impl.Original };
}

MODIODLL_EXPORT CModioString* GetModioAvatarURL50x50(const CModioAvatar* ModioAvatar)
{
	return new CModioString{ ModioAvatar->Impl.Thumb50x50 };
}

MODIODLL_EXPORT CModioString* GetModioAvatarURL100x100(const CModioAvatar* ModioAvatar)
{
	return new CModioString{ ModioAvatar->Impl.Thumb100x100 };
}


MODIODLL_EXPORT void SetModioAvatarFilename(CModioAvatar* Item, const CModioString* Filename)
{
	if (Item != nullptr)
	{
		Item->Impl.Filename = Modio::GetImpl(Filename);
	}
}
MODIODLL_EXPORT void SetModioAvatarFullSizeURL(CModioAvatar* Item, const CModioString* FullSizeURL)
{
	if (Item != nullptr)
	{
		Item->Impl.Original = Modio::GetImpl(FullSizeURL);
	}
}
MODIODLL_EXPORT void SetModioAvatarURL50x50(CModioAvatar* Item, const CModioString* URL50x50)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb50x50 = Modio::GetImpl(URL50x50);
	}
}
MODIODLL_EXPORT void SetModioAvatarURL100x100(CModioAvatar* Item, const CModioString* URL100x100)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb100x100 = Modio::GetImpl(URL100x100);
	}
}

MODIODLL_EXPORT CModioFieldError* CreateModioFieldError()
{
	return new CModioFieldError{ Modio::FieldError {} };
}

MODIODLL_EXPORT void ReleaseModioFieldError(CModioFieldError* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioFieldError* CopyModioFieldError(const CModioFieldError* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioFieldError { *Item };
}



MODIODLL_EXPORT CModioValidationError* CreateModioValidationError()
{
	return new CModioValidationError{ Modio::FieldError {} };
}

MODIODLL_EXPORT void ReleaseModioValidationError(CModioValidationError* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioValidationError* CopyModioValidationError(const CModioValidationError* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioValidationError { *Item };
}

MODIODLL_EXPORT CModioString* GetModioValidationErrorFieldName(const CModioValidationError* ValidationError)
{
	return new CModioString{ ValidationError->Impl.Field };
}

MODIODLL_EXPORT CModioString* GetModioValidationErrorError(const CModioValidationError* ValidationError)
{
	return new CModioString{ ValidationError->Impl.Error };
}


MODIODLL_EXPORT void SetModioValidationErrorFieldName(CModioValidationError* Item, const CModioString* FieldName)
{
	if (Item != nullptr)
	{
		Item->Impl.Field = Modio::GetImpl(FieldName);
	}
}
MODIODLL_EXPORT void SetModioValidationErrorError(CModioValidationError* Item, const CModioString* Error)
{
	if (Item != nullptr)
	{
		Item->Impl.Error = Modio::GetImpl(Error);
	}
}

MODIODLL_EXPORT CModioValidationErrorList* CreateModioValidationErrorList()
{
	return new CModioValidationErrorList{ std::vector<Modio::FieldError> {} };
}

MODIODLL_EXPORT void ReleaseModioValidationErrorList(CModioValidationErrorList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioValidationErrorList* CopyModioValidationErrorList(const CModioValidationErrorList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioValidationErrorList { *Item };
}


MODIODLL_EXPORT CModioValidationError* GetModioValidationErrorListErrorByIndex(const CModioValidationErrorList* ValidationErrorList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ValidationErrorList).size())
	{
		return new CModioValidationError{ Modio::GetImpl(ValidationErrorList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioValidationErrorListErrorByIndex(CModioValidationErrorList* ValidationErrorList, uint64_t Index, const CModioValidationError * Value)
{
	if (Index < Modio::GetImpl(ValidationErrorList).size())
	{
			Modio::GetImpl(ValidationErrorList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioValidationErrorListCount(const CModioValidationErrorList* ValidationErrorList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ValidationErrorList).size());
}

MODIODLL_EXPORT void SetModioValidationErrorListCount(CModioValidationErrorList* ValidationErrorList, uint64_t Count)
{
	Modio::GetImpl(ValidationErrorList).resize(Count);
}


MODIODLL_EXPORT CModioModProgressInfo* CreateModioModProgressInfo(const CModioModID* ID)
{
return new CModioModProgressInfo{ Modio::ModProgressInfo { Modio::GetImpl(ID) } };
}

MODIODLL_EXPORT void ReleaseModioModProgressInfo(CModioModProgressInfo* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModProgressInfo* CopyModioModProgressInfo(const CModioModProgressInfo* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModProgressInfo { *Item };
}

MODIODLL_EXPORT CModioModID* GetModioModProgressInfoID(const CModioModProgressInfo* ProgressInfo)
{
	return new CModioModID{ ProgressInfo->Impl.ID };
}


MODIODLL_EXPORT void SetModioModProgressInfoID(CModioModProgressInfo* Item, const CModioModID* ID)
{
	if (Item != nullptr)
	{
		Item->Impl.ID = Modio::GetImpl(ID);
	}
}

MODIODLL_EXPORT CModioStringList* CreateModioStringList()
{
	return new CModioStringList{ std::vector<std::string> {} };
}

MODIODLL_EXPORT void ReleaseModioStringList(CModioStringList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioStringList* CopyModioStringList(const CModioStringList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioStringList { *Item };
}


MODIODLL_EXPORT CModioString* GetModioStringListStringByIndex(const CModioStringList* ModioStringList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioStringList).size())
	{
		return new CModioString{ Modio::GetImpl(ModioStringList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioStringListStringByIndex(CModioStringList* ModioStringList, uint64_t Index, const CModioString * Value)
{
	if (Index < Modio::GetImpl(ModioStringList).size())
	{
			Modio::GetImpl(ModioStringList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioStringListCount(const CModioStringList* ModioStringList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioStringList).size());
}

MODIODLL_EXPORT void SetModioStringListCount(CModioStringList* ModioStringList, uint64_t Count)
{
	Modio::GetImpl(ModioStringList).resize(Count);
}


MODIODLL_EXPORT CModioStringMap* CreateModioStringMap()
{
	return new CModioStringMap{ std::map<std::string, std::string> {} };
}

MODIODLL_EXPORT void ReleaseModioStringMap(CModioStringMap* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioStringMap* CopyModioStringMap(const CModioStringMap* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioStringMap { *Item };
}


MODIODLL_EXPORT const char* ModioStringMapGetKeyValue(CModioStringMap* Item, const char* Key)
{
	if(Modio::GetImpl(Item).count(Key))
	{
		return Modio::GetImpl(Item)[Key].c_str();
	}
	return nullptr;
}

MODIODLL_EXPORT CModioStringMapIterator* ModioStringMapBegin(CModioStringMap* Item)
{ 
	if(Modio::GetImpl(Item).begin() == Modio::GetImpl(Item).end())
		return nullptr;
	return new CModioStringMapIterator{ Modio::GetImpl(Item).begin(), Modio::GetImpl(Item).end() };
}

MODIODLL_EXPORT void ReleaseModioStringMapIterator(CModioStringMapIterator* Item)
{
	delete Item;
}

MODIODLL_EXPORT bool ModioStringMapNext(CModioStringMapIterator* Iterator)
{
	return (++Iterator->Iter) != Iterator->End;
}

MODIODLL_EXPORT const char* ModioStringMapGetKey(CModioStringMapIterator* Iterator)
{
	return Iterator->Iter->first.c_str();
}

MODIODLL_EXPORT const char* ModioStringMapGetValue(CModioStringMapIterator* Iterator)
{
	return Iterator->Iter->second.c_str();
}

MODIODLL_EXPORT void ModioStringMapSetKeyValue(CModioStringMap* Item, const char* Key, const char* Value)
{
	Modio::GetImpl(Item)[Key] = Value;
}


MODIODLL_EXPORT CModioAuthenticationParams* CreateModioAuthenticationParams(const char* OAuthTokenData, size_t OAuthTokenLength)
{
return new CModioAuthenticationParams{ Modio::AuthenticationParams { std::string { OAuthTokenData, OAuthTokenLength } } };
}

MODIODLL_EXPORT void ReleaseModioAuthenticationParams(CModioAuthenticationParams* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioAuthenticationParams* CopyModioAuthenticationParams(const CModioAuthenticationParams* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioAuthenticationParams { *Item };
}

MODIODLL_EXPORT CModioString* GetModioAuthenticationParamsUserEmail(const CModioAuthenticationParams* AuthenticationParams)
{
	if (AuthenticationParams->Impl.UserEmail.has_value())
	{
		return new CModioString{ *(AuthenticationParams->Impl.UserEmail) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT bool GetModioAuthenticationParamsUserHasAcceptedTerms(const CModioAuthenticationParams* AuthenticationParams)
{
	return bool{ AuthenticationParams->Impl.bUserHasAcceptedTerms };
}

MODIODLL_EXPORT bool GetModioAuthenticationParamsURLEncodeAuthToken(const CModioAuthenticationParams* AuthenticationParams)
{
	return bool{ AuthenticationParams->Impl.bURLEncodeAuthToken };
}

MODIODLL_EXPORT CModioStringMap* GetModioAuthenticationParamsExtendedParameters(const CModioAuthenticationParams* AuthenticationParams)
{
	return new CModioStringMap{ AuthenticationParams->Impl.ExtendedParameters };
}


MODIODLL_EXPORT void SetModioAuthenticationParamsUserEmail(CModioAuthenticationParams* Item, const CModioString* UserEmail)
{
	if (Item != nullptr)
	{
		if(UserEmail)
		{
			Item->Impl.UserEmail = Modio::GetImpl(UserEmail);
		}
		else if(Item->Impl.UserEmail.has_value())
		{
			Item->Impl.UserEmail.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioAuthenticationParamsUserHasAcceptedTerms(CModioAuthenticationParams* Item, const bool UserHasAcceptedTerms)
{
	if (Item != nullptr)
	{
		Item->Impl.bUserHasAcceptedTerms = UserHasAcceptedTerms;
	}
}
MODIODLL_EXPORT void SetModioAuthenticationParamsURLEncodeAuthToken(CModioAuthenticationParams* Item, const bool URLEncodeAuthToken)
{
	if (Item != nullptr)
	{
		Item->Impl.bURLEncodeAuthToken = URLEncodeAuthToken;
	}
}
MODIODLL_EXPORT void SetModioAuthenticationParamsExtendedParameters(CModioAuthenticationParams* Item, const CModioStringMap* ExtendedParameters)
{
	if (Item != nullptr)
	{
		Item->Impl.ExtendedParameters = Modio::GetImpl(ExtendedParameters);
	}
}

MODIODLL_EXPORT CModioModInfoList* CreateModioModInfoList()
{
	return new CModioModInfoList{ Modio::List<std::vector, Modio::ModInfo> {} };
}

MODIODLL_EXPORT void ReleaseModioModInfoList(CModioModInfoList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModInfoList* CopyModioModInfoList(const CModioModInfoList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModInfoList { *Item };
}


MODIODLL_EXPORT CModioModInfo* GetModioModInfoListModInfoByIndex(const CModioModInfoList* ModioModInfoList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioModInfoList).Size())
	{
		return new CModioModInfo{ Modio::GetImpl(ModioModInfoList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioModInfoListModInfoByIndex(CModioModInfoList* ModioModInfoList, uint64_t Index, const CModioModInfo * Value)
{
	if (Index < Modio::GetImpl(ModioModInfoList).Size())
	{
			Modio::GetImpl(ModioModInfoList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioModInfoListCount(const CModioModInfoList* ModioModInfoList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioModInfoList).Size());
}

MODIODLL_EXPORT void SetModioModInfoListCount(CModioModInfoList* ModioModInfoList, uint64_t Count)
{
	Modio::GetImpl(ModioModInfoList).GetRawList().resize(Count);
}


MODIODLL_EXPORT CModioModCollectionInfoList* CreateModioModCollectionInfoList()
{
	return new CModioModCollectionInfoList{ Modio::List<std::vector, Modio::ModCollectionInfo> {} };
}

MODIODLL_EXPORT void ReleaseModioModCollectionInfoList(CModioModCollectionInfoList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModCollectionInfoList* CopyModioModCollectionInfoList(const CModioModCollectionInfoList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModCollectionInfoList { *Item };
}


MODIODLL_EXPORT CModioModCollectionInfo* GetModioModCollectionInfoListModCollectionInfoByIndex(const CModioModCollectionInfoList* ModioModCollectionInfoList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioModCollectionInfoList).Size())
	{
		return new CModioModCollectionInfo{ Modio::GetImpl(ModioModCollectionInfoList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioModCollectionInfoListModCollectionInfoByIndex(CModioModCollectionInfoList* ModioModCollectionInfoList, uint64_t Index, const CModioModCollectionInfo * Value)
{
	if (Index < Modio::GetImpl(ModioModCollectionInfoList).Size())
	{
			Modio::GetImpl(ModioModCollectionInfoList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioModCollectionInfoListCount(const CModioModCollectionInfoList* ModioModCollectionInfoList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioModCollectionInfoList).Size());
}

MODIODLL_EXPORT void SetModioModCollectionInfoListCount(CModioModCollectionInfoList* ModioModCollectionInfoList, uint64_t Count)
{
	Modio::GetImpl(ModioModCollectionInfoList).GetRawList().resize(Count);
}


MODIODLL_EXPORT CModioUserList* CreateModioUserList()
{
	return new CModioUserList{ Modio::List<std::vector, Modio::User> {} };
}

MODIODLL_EXPORT void ReleaseModioUserList(CModioUserList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioUserList* CopyModioUserList(const CModioUserList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioUserList { *Item };
}


MODIODLL_EXPORT CModioUser* GetModioUserListUserByIndex(const CModioUserList* ModioUserList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioUserList).Size())
	{
		return new CModioUser{ Modio::GetImpl(ModioUserList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioUserListUserByIndex(CModioUserList* ModioUserList, uint64_t Index, const CModioUser * Value)
{
	if (Index < Modio::GetImpl(ModioUserList).Size())
	{
			Modio::GetImpl(ModioUserList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioUserListCount(const CModioUserList* ModioUserList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioUserList).Size());
}

MODIODLL_EXPORT void SetModioUserListCount(CModioUserList* ModioUserList, uint64_t Count)
{
	Modio::GetImpl(ModioUserList).GetRawList().resize(Count);
}


MODIODLL_EXPORT CModioGamePlatform* CreateModioGamePlatform()
{
	return new CModioGamePlatform{ Modio::GamePlatform {} };
}

MODIODLL_EXPORT void ReleaseModioGamePlatform(CModioGamePlatform* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioGamePlatform* CopyModioGamePlatform(const CModioGamePlatform* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioGamePlatform { *Item };
}

MODIODLL_EXPORT EModioModfilePlatform GetModioGamePlatformPlatform(const CModioGamePlatform* GamePlatform)
{
	return ConvertEnumToC( GamePlatform->Impl.Platform );
}

MODIODLL_EXPORT bool GetModioGamePlatformLocked(const CModioGamePlatform* GamePlatform)
{
	return GamePlatform->Impl.Locked;
}

MODIODLL_EXPORT bool GetModioGamePlatformModerated(const CModioGamePlatform* GamePlatform)
{
	return GamePlatform->Impl.Moderated;
}


MODIODLL_EXPORT void SetModioGamePlatformPlatform(CModioGamePlatform* Item, const EModioModfilePlatform Platform)
{
	if (Item != nullptr)
	{
		Item->Impl.Platform = static_cast<Modio::ModfilePlatform>(Platform);
	}
}
MODIODLL_EXPORT void SetModioGamePlatformLocked(CModioGamePlatform* Item, const bool Locked)
{
	if (Item != nullptr)
	{
		Item->Impl.Locked = Locked;
	}
}
MODIODLL_EXPORT void SetModioGamePlatformModerated(CModioGamePlatform* Item, const bool Moderated)
{
	if (Item != nullptr)
	{
		Item->Impl.Moderated = Moderated;
	}
}

MODIODLL_EXPORT CModioModTagInfo* CreateModioModTagInfo()
{
	return new CModioModTagInfo{ Modio::ModTagInfo {} };
}

MODIODLL_EXPORT void ReleaseModioModTagInfo(CModioModTagInfo* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModTagInfo* CopyModioModTagInfo(const CModioModTagInfo* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModTagInfo { *Item };
}

MODIODLL_EXPORT CModioString* GetModioModTagInfoTagGroupName(const CModioModTagInfo* ModioModTagInfo)
{
	return new CModioString{ ModioModTagInfo->Impl.TagGroupName };
}

MODIODLL_EXPORT CModioStringList* GetModioModTagInfoTagGroupValues(const CModioModTagInfo* ModioModTagInfo)
{
	return new CModioStringList{ ModioModTagInfo->Impl.TagGroupValues };
}

MODIODLL_EXPORT bool GetModioModTagInfoAllowMultipleSelection(const CModioModTagInfo* ModioModTagInfo)
{
	return ModioModTagInfo->Impl.bAllowMultipleSelection;
}


MODIODLL_EXPORT void SetModioModTagInfoTagGroupName(CModioModTagInfo* Item, const CModioString* TagGroupName)
{
	if (Item != nullptr)
	{
		Item->Impl.TagGroupName = Modio::GetImpl(TagGroupName);
	}
}
MODIODLL_EXPORT void SetModioModTagInfoTagGroupValues(CModioModTagInfo* Item, const CModioStringList* TagGroupValues)
{
	if (Item != nullptr)
	{
		Item->Impl.TagGroupValues = Modio::GetImpl(TagGroupValues);
	}
}
MODIODLL_EXPORT void SetModioModTagInfoAllowMultipleSelection(CModioModTagInfo* Item, const bool AllowMultipleSelection)
{
	if (Item != nullptr)
	{
		Item->Impl.bAllowMultipleSelection = AllowMultipleSelection;
	}
}

MODIODLL_EXPORT CModioModTagOptions* CreateModioModTagOptions()
{
	return new CModioModTagOptions{ Modio::List<std::vector, Modio::ModTagInfo> {} };
}

MODIODLL_EXPORT void ReleaseModioModTagOptions(CModioModTagOptions* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModTagOptions* CopyModioModTagOptions(const CModioModTagOptions* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModTagOptions { *Item };
}


MODIODLL_EXPORT CModioModTagInfo* GetModioModTagOptionsModTagInfoByIndex(const CModioModTagOptions* ModioModTagOptions, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioModTagOptions).Size())
	{
		return new CModioModTagInfo{ Modio::GetImpl(ModioModTagOptions)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioModTagOptionsModTagInfoByIndex(CModioModTagOptions* ModioModTagOptions, uint64_t Index, const CModioModTagInfo * Value)
{
	if (Index < Modio::GetImpl(ModioModTagOptions).Size())
	{
			Modio::GetImpl(ModioModTagOptions)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioModTagOptionsCount(const CModioModTagOptions* ModioModTagOptions)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioModTagOptions).Size());
}

MODIODLL_EXPORT void SetModioModTagOptionsCount(CModioModTagOptions* ModioModTagOptions, uint64_t Count)
{
	Modio::GetImpl(ModioModTagOptions).GetRawList().resize(Count);
}


MODIODLL_EXPORT CModioModTagInfoList* CreateModioModTagInfoList()
{
	return new CModioModTagInfoList{ std::vector<Modio::ModTagInfo> {} };
}

MODIODLL_EXPORT void ReleaseModioModTagInfoList(CModioModTagInfoList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModTagInfoList* CopyModioModTagInfoList(const CModioModTagInfoList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModTagInfoList { *Item };
}


MODIODLL_EXPORT CModioModTagInfo* GetModioModTagInfoListModTagInfoByIndex(const CModioModTagInfoList* ModioModTagInfoList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioModTagInfoList).size())
	{
		return new CModioModTagInfo{ Modio::GetImpl(ModioModTagInfoList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioModTagInfoListModTagInfoByIndex(CModioModTagInfoList* ModioModTagInfoList, uint64_t Index, const CModioModTagInfo * Value)
{
	if (Index < Modio::GetImpl(ModioModTagInfoList).size())
	{
			Modio::GetImpl(ModioModTagInfoList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioModTagInfoListCount(const CModioModTagInfoList* ModioModTagInfoList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioModTagInfoList).size());
}

MODIODLL_EXPORT void SetModioModTagInfoListCount(CModioModTagInfoList* ModioModTagInfoList, uint64_t Count)
{
	Modio::GetImpl(ModioModTagInfoList).resize(Count);
}


MODIODLL_EXPORT CModioModCollectionMap* CreateModioModCollectionMap()
{
	return new CModioModCollectionMap{ std::map<Modio::ModID, Modio::ModCollectionEntry> {} };
}

MODIODLL_EXPORT void ReleaseModioModCollectionMap(CModioModCollectionMap* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModCollectionMap* CopyModioModCollectionMap(const CModioModCollectionMap* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModCollectionMap { *Item };
}


MODIODLL_EXPORT CModioModCollectionEntry* ModioModCollectionMapGetKeyValue(CModioModCollectionMap* Item, CModioModID const* Key)
{
	if(Modio::GetImpl(Item).count(Modio::GetImpl(Key)))
	{
		return new CModioModCollectionEntry { Modio::GetImpl(Item)[Modio::GetImpl(Key)] } ;
	}
	return nullptr;
}

MODIODLL_EXPORT CModioModCollectionMapIterator* ModioModCollectionMapBegin(CModioModCollectionMap* Item)
{ 
	if(Modio::GetImpl(Item).begin() == Modio::GetImpl(Item).end())
		return nullptr;
	return new CModioModCollectionMapIterator{ Modio::GetImpl(Item).begin(), Modio::GetImpl(Item).end() };
}

MODIODLL_EXPORT void ReleaseModioModCollectionMapIterator(CModioModCollectionMapIterator* Item)
{
	delete Item;
}

MODIODLL_EXPORT bool ModioModCollectionMapNext(CModioModCollectionMapIterator* Iterator)
{
	return (++Iterator->Iter) != Iterator->End;
}

MODIODLL_EXPORT CModioModID* ModioModCollectionMapGetKey(CModioModCollectionMapIterator* Iterator)
{
	return new CModioModID{ Iterator->Iter->first };
}

MODIODLL_EXPORT CModioModCollectionEntry* ModioModCollectionMapGetValue(CModioModCollectionMapIterator* Iterator)
{
	return new CModioModCollectionEntry{ Iterator->Iter->second };
}

MODIODLL_EXPORT void ModioModCollectionMapSetKeyValue(CModioModCollectionMap* Item, CModioModID const *Key, CModioModCollectionEntry const *Value)
{
	Modio::GetImpl(Item)[Modio::GetImpl(Key)] = Modio::GetImpl(Value);
}


MODIODLL_EXPORT CModioModDependency* CreateModioModDependency()
{
	return new CModioModDependency{ Modio::ModDependency {} };
}

MODIODLL_EXPORT void ReleaseModioModDependency(CModioModDependency* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModDependency* CopyModioModDependency(const CModioModDependency* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModDependency { *Item };
}

MODIODLL_EXPORT CModioModID* GetModioModDependencyModID(const CModioModDependency* ModioModDependency)
{
	return new CModioModID{ ModioModDependency->Impl.ModID };
}

MODIODLL_EXPORT CModioString* GetModioModDependencyModName(const CModioModDependency* ModioModDependency)
{
	return new CModioString{ ModioModDependency->Impl.ModName };
}

MODIODLL_EXPORT int64_t GetModioModDependencyDateAdded(const CModioModDependency* ModioModDependency)
{
	return int64_t{ ModioModDependency->Impl.DateAdded };
}

MODIODLL_EXPORT int64_t GetModioModDependencyDateUpdated(const CModioModDependency* ModioModDependency)
{
	return int64_t{ ModioModDependency->Impl.DateUpdated };
}

MODIODLL_EXPORT uint8_t GetModioModDependencyDependencyDepth(const CModioModDependency* ModioModDependency)
{
	return uint8_t{ ModioModDependency->Impl.DependencyDepth };
}

MODIODLL_EXPORT CModioLogo* GetModioModDependencyLogo(const CModioModDependency* ModioModDependency)
{
	return new CModioLogo{ ModioModDependency->Impl.Logo };
}

MODIODLL_EXPORT CModioFileInfo* GetModioModDependencyFileInfo(const CModioModDependency* ModioModDependency)
{
	if (ModioModDependency->Impl.FileInfo.has_value())
	{
		return new CModioFileInfo{ *(ModioModDependency->Impl.FileInfo) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT EModioModServerSideStatus GetModioModDependencyStatus(const CModioModDependency* ModioModDependency)
{
	return ConvertEnumToC( ModioModDependency->Impl.Status );
}

MODIODLL_EXPORT EModioObjectVisibility GetModioModDependencyVisibility(const CModioModDependency* ModioModDependency)
{
	return ConvertEnumToC( ModioModDependency->Impl.Visibility );
}


MODIODLL_EXPORT void SetModioModDependencyModID(CModioModDependency* Item, const CModioModID* ModID)
{
	if (Item != nullptr)
	{
		Item->Impl.ModID = Modio::GetImpl(ModID);
	}
}
MODIODLL_EXPORT void SetModioModDependencyModName(CModioModDependency* Item, const CModioString* ModName)
{
	if (Item != nullptr)
	{
		Item->Impl.ModName = Modio::GetImpl(ModName);
	}
}
MODIODLL_EXPORT void SetModioModDependencyDateAdded(CModioModDependency* Item, const int64_t DateAdded)
{
	if (Item != nullptr)
	{
		Item->Impl.DateAdded = DateAdded;
	}
}
MODIODLL_EXPORT void SetModioModDependencyDateUpdated(CModioModDependency* Item, const int64_t DateUpdated)
{
	if (Item != nullptr)
	{
		Item->Impl.DateUpdated = DateUpdated;
	}
}
MODIODLL_EXPORT void SetModioModDependencyDependencyDepth(CModioModDependency* Item, const uint8_t DependencyDepth)
{
	if (Item != nullptr)
	{
		Item->Impl.DependencyDepth = DependencyDepth;
	}
}
MODIODLL_EXPORT void SetModioModDependencyLogo(CModioModDependency* Item, const CModioLogo* Logo)
{
	if (Item != nullptr)
	{
		Item->Impl.Logo = Modio::GetImpl(Logo);
	}
}
MODIODLL_EXPORT void SetModioModDependencyFileInfo(CModioModDependency* Item, const CModioFileInfo* FileInfo)
{
	if (Item != nullptr)
	{
		if(FileInfo)
		{
			Item->Impl.FileInfo = Modio::GetImpl(FileInfo);
		}
		else if(Item->Impl.FileInfo.has_value())
		{
			Item->Impl.FileInfo.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioModDependencyStatus(CModioModDependency* Item, const EModioModServerSideStatus Status)
{
	if (Item != nullptr)
	{
		Item->Impl.Status = static_cast<Modio::ModServerSideStatus>(Status);
	}
}
MODIODLL_EXPORT void SetModioModDependencyVisibility(CModioModDependency* Item, const EModioObjectVisibility Visibility)
{
	if (Item != nullptr)
	{
		Item->Impl.Visibility = static_cast<Modio::ObjectVisibility>(Visibility);
	}
}

MODIODLL_EXPORT CModioModDependencyList* CreateModioModDependencyList()
{
	return new CModioModDependencyList{ Modio::ModDependencyList {} };
}

MODIODLL_EXPORT void ReleaseModioModDependencyList(CModioModDependencyList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModDependencyList* CopyModioModDependencyList(const CModioModDependencyList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModDependencyList { *Item };
}


MODIODLL_EXPORT CModioModDependency* GetModioModDependencyListDependencyByIndex(const CModioModDependencyList* ModDependencyList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModDependencyList).Size())
	{
		return new CModioModDependency{ Modio::GetImpl(ModDependencyList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioModDependencyListDependencyByIndex(CModioModDependencyList* ModDependencyList, uint64_t Index, const CModioModDependency * Value)
{
	if (Index < Modio::GetImpl(ModDependencyList).Size())
	{
			Modio::GetImpl(ModDependencyList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioModDependencyListCount(const CModioModDependencyList* ModDependencyList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModDependencyList).Size());
}

MODIODLL_EXPORT void SetModioModDependencyListCount(CModioModDependencyList* ModDependencyList, uint64_t Count)
{
	Modio::GetImpl(ModDependencyList).GetRawList().resize(Count);
}


MODIODLL_EXPORT CModioModCollectionEntry* CreateModioModCollectionEntry()
{
	return new CModioModCollectionEntry{ Modio::ModCollectionEntry {} };
}

MODIODLL_EXPORT void ReleaseModioModCollectionEntry(CModioModCollectionEntry* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModCollectionEntry* CopyModioModCollectionEntry(const CModioModCollectionEntry* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModCollectionEntry { *Item };
}

MODIODLL_EXPORT EModioModState GetModioModCollectionEntryModState(const CModioModCollectionEntry* ModioModCollectionEntry)
{
	return ConvertEnumToC( ModioModCollectionEntry->Impl.GetModState() );
}

MODIODLL_EXPORT CModioModID* GetModioModCollectionEntryModID(const CModioModCollectionEntry* ModioModCollectionEntry)
{
	return new CModioModID{ ModioModCollectionEntry->Impl.GetID() };
}

MODIODLL_EXPORT CModioModInfo* GetModioModCollectionEntryModInfo(const CModioModCollectionEntry* ModioModCollectionEntry)
{
	return new CModioModInfo{ ModioModCollectionEntry->Impl.GetModProfile() };
}

MODIODLL_EXPORT CModioString* GetModioModCollectionEntryPath(const CModioModCollectionEntry* ModioModCollectionEntry)
{
	return new CModioString{ ModioModCollectionEntry->Impl.GetPath() };
}

MODIODLL_EXPORT COptionalUInt64 GetModioModCollectionEntrySizeOnDisk(const CModioModCollectionEntry* ModioModCollectionEntry)
{
	if (ModioModCollectionEntry->Impl.GetSizeOnDisk().has_value())
	{
		return COptionalUInt64{ *(ModioModCollectionEntry->Impl.GetSizeOnDisk()), true };
;
	}
	else
	{
		return { 0, false };
	}
}



MODIODLL_EXPORT CModioImageList* CreateModioImageList()
{
	return new CModioImageList{ Modio::List<std::vector, Modio::Detail::Image> {} };
}

MODIODLL_EXPORT void ReleaseModioImageList(CModioImageList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioImageList* CopyModioImageList(const CModioImageList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioImageList { *Item };
}


MODIODLL_EXPORT CModioImage* GetModioImageListImageByIndex(const CModioImageList* ModioImageList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioImageList).Size())
	{
		return new CModioImage{ Modio::GetImpl(ModioImageList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioImageListImageByIndex(CModioImageList* ModioImageList, uint64_t Index, const CModioImage * Value)
{
	if (Index < Modio::GetImpl(ModioImageList).Size())
	{
			Modio::GetImpl(ModioImageList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioImageListCount(const CModioImageList* ModioImageList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioImageList).Size());
}

MODIODLL_EXPORT void SetModioImageListCount(CModioImageList* ModioImageList, uint64_t Count)
{
	Modio::GetImpl(ModioImageList).GetRawList().resize(Count);
}


MODIODLL_EXPORT CModioErrorCode* CreateModioErrorCode()
{
	return new CModioErrorCode{ Modio::ErrorCode {} };
}

MODIODLL_EXPORT void ReleaseModioErrorCode(CModioErrorCode* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioErrorCode* CopyModioErrorCode(const CModioErrorCode* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioErrorCode { *Item };
}

MODIODLL_EXPORT bool AreEqualModioErrorCode(const CModioErrorCode* First, const CModioErrorCode* Second)
{
	return Modio::GetImpl(First) == Modio::GetImpl(Second);
}

MODIODLL_EXPORT CModioString* GetModioErrorCodeMessage(const CModioErrorCode* ModioErrorCode)
{
	return new CModioString{ ModioErrorCode->Impl.message() };
}

MODIODLL_EXPORT bool GetModioErrorCodeIsError(const CModioErrorCode* ModioErrorCode)
{
	return bool{ ModioErrorCode->Impl };
}



MODIODLL_EXPORT CModioAPIKey* CreateModioAPIKey(const char* Data, size_t Length)
{
return new CModioAPIKey{ Modio::ApiKey { std::string { Data, Length } } };
}

MODIODLL_EXPORT void ReleaseModioAPIKey(CModioAPIKey* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioAPIKey* CopyModioAPIKey(const CModioAPIKey* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioAPIKey { *Item };
}



MODIODLL_EXPORT CModioModInfo* CreateModioModInfo()
{
	return new CModioModInfo{ Modio::ModInfo {} };
}

MODIODLL_EXPORT void ReleaseModioModInfo(CModioModInfo* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModInfo* CopyModioModInfo(const CModioModInfo* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModInfo { *Item };
}

MODIODLL_EXPORT CModioModID* GetModioModInfoModID(const CModioModInfo* ModInfo)
{
	return new CModioModID{ ModInfo->Impl.ModId };
}

MODIODLL_EXPORT CModioString* GetModioModInfoProfileName(const CModioModInfo* ModInfo)
{
	return new CModioString{ ModInfo->Impl.ProfileName };
}

MODIODLL_EXPORT CModioString* GetModioModInfoProfileSummary(const CModioModInfo* ModInfo)
{
	return new CModioString{ ModInfo->Impl.ProfileSummary };
}

MODIODLL_EXPORT CModioString* GetModioModInfoProfileDescription(const CModioModInfo* ModInfo)
{
	return new CModioString{ ModInfo->Impl.ProfileDescription };
}

MODIODLL_EXPORT CModioString* GetModioModInfoProfileDescriptionPlaintext(const CModioModInfo* ModInfo)
{
	return new CModioString{ ModInfo->Impl.ProfileDescriptionPlaintext };
}

MODIODLL_EXPORT CModioString* GetModioModInfoProfileURL(const CModioModInfo* ModInfo)
{
	return new CModioString{ ModInfo->Impl.ProfileURL };
}

MODIODLL_EXPORT CModioUser* GetModioModInfoProfileSubmittedBy(const CModioModInfo* ModInfo)
{
	return new CModioUser{ ModInfo->Impl.ProfileSubmittedBy };
}

MODIODLL_EXPORT int64_t GetModioModInfoProfileDateAdded(const CModioModInfo* ModInfo)
{
	return int64_t{ ModInfo->Impl.ProfileDateAdded };
}

MODIODLL_EXPORT int64_t GetModioModInfoProfileDateUpdated(const CModioModInfo* ModInfo)
{
	return int64_t{ ModInfo->Impl.ProfileDateUpdated };
}

MODIODLL_EXPORT int64_t GetModioModInfoProfileDateLive(const CModioModInfo* ModInfo)
{
	return int64_t{ ModInfo->Impl.ProfileDateLive };
}

MODIODLL_EXPORT CModioMaturityProfile GetModioModInfoProfileMaturityFlags(const CModioModInfo* ModInfo)
{
	return CModioMaturityProfile(ModInfo->Impl.ProfileMaturityOption.RawValue());
}

MODIODLL_EXPORT CModioString* GetModioModInfoMetadataBlob(const CModioModInfo* ModInfo)
{
	return new CModioString{ ModInfo->Impl.MetadataBlob };
}

MODIODLL_EXPORT CModioFileInfo* GetModioModInfoFileInfo(const CModioModInfo* ModInfo)
{
	if (ModInfo->Impl.FileInfo.has_value())
	{
		return new CModioFileInfo{ *(ModInfo->Impl.FileInfo) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioMetadataKVP* GetModioModInfoMetadataKVP(const CModioModInfo* ModInfo)
{
	return new CModioMetadataKVP{ ModInfo->Impl.MetadataKvp };
}

MODIODLL_EXPORT CModioModTagList* GetModioModInfoTags(const CModioModInfo* ModInfo)
{
	return new CModioModTagList{ ModInfo->Impl.Tags };
}

MODIODLL_EXPORT size_t GetModioModInfoNumGalleryImages(const CModioModInfo* ModInfo)
{
	return size_t{ ModInfo->Impl.NumGalleryImages };
}

MODIODLL_EXPORT CModioImageList* GetModioModInfoGallery(const CModioModInfo* ModInfo)
{
	return new CModioImageList{ ModInfo->Impl.GalleryImages };
}

MODIODLL_EXPORT CModioStringList* GetModioModInfoYoutubeURLs(const CModioModInfo* ModInfo)
{
	return new CModioStringList{ ModInfo->Impl.YoutubeURLs.GetRawList() };
}

MODIODLL_EXPORT CModioStringList* GetModioModInfoSketchfabURLs(const CModioModInfo* ModInfo)
{
	return new CModioStringList{ ModInfo->Impl.SketchfabURLs.GetRawList() };
}

MODIODLL_EXPORT CModioModStats* GetModioModInfoStats(const CModioModInfo* ModInfo)
{
	return new CModioModStats{ ModInfo->Impl.Stats };
}

MODIODLL_EXPORT CModioModLogo* GetModioModInfoLogo(const CModioModInfo* ModInfo)
{
	return new CModioModLogo{ ModInfo->Impl.ModLogo };
}

MODIODLL_EXPORT CModioString* GetModioModInfoVersion(const CModioModInfo* ModInfo)
{
	return new CModioString{ ModInfo->Impl.Version };
}

MODIODLL_EXPORT EModioModServerSideStatus GetModioModInfoServerStatus(const CModioModInfo* ModInfo)
{
	return ConvertEnumToC( ModInfo->Impl.ModStatus );
}

MODIODLL_EXPORT EModioObjectVisibility GetModioModInfoVisibility(const CModioModInfo* ModInfo)
{
	return ConvertEnumToC( ModInfo->Impl.Visibility );
}

MODIODLL_EXPORT uint64_t GetModioModInfoPrice(const CModioModInfo* ModInfo)
{
	return uint64_t{ ModInfo->Impl.Price };
}

MODIODLL_EXPORT bool GetModioModInfoDependencies(const CModioModInfo* ModInfo)
{
	return bool{ ModInfo->Impl.Dependencies };
}

MODIODLL_EXPORT CModioModCommunityOptionsFlags GetModioModInfoCommunityOptions(const CModioModInfo* ModioModInfo)
{
	return CModioModCommunityOptionsFlags(ModioModInfo->Impl.CommunityOptions.RawValue());
}


MODIODLL_EXPORT void SetModioModInfoModID(CModioModInfo* Item, const CModioModID* ModID)
{
	if (Item != nullptr)
	{
		Item->Impl.ModId = Modio::GetImpl(ModID);
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileName(CModioModInfo* Item, const CModioString* ProfileName)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileName = Modio::GetImpl(ProfileName);
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileSummary(CModioModInfo* Item, const CModioString* ProfileSummary)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileSummary = Modio::GetImpl(ProfileSummary);
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileDescription(CModioModInfo* Item, const CModioString* ProfileDescription)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDescription = Modio::GetImpl(ProfileDescription);
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileDescriptionPlaintext(CModioModInfo* Item, const CModioString* ProfileDescriptionPlaintext)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDescriptionPlaintext = Modio::GetImpl(ProfileDescriptionPlaintext);
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileURL(CModioModInfo* Item, const CModioString* ProfileURL)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileURL = Modio::GetImpl(ProfileURL);
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileSubmittedBy(CModioModInfo* Item, const CModioUser* ProfileSubmittedBy)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileSubmittedBy = Modio::GetImpl(ProfileSubmittedBy);
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileDateAdded(CModioModInfo* Item, const int64_t ProfileDateAdded)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDateAdded = ProfileDateAdded;
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileDateUpdated(CModioModInfo* Item, const int64_t ProfileDateUpdated)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDateUpdated = ProfileDateUpdated;
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileDateLive(CModioModInfo* Item, const int64_t ProfileDateLive)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDateLive = ProfileDateLive;
	}
}
MODIODLL_EXPORT void SetModioModInfoProfileMaturityFlags(CModioModInfo* Item, CModioMaturityProfile ProfileMaturityFlags)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileMaturityOption = static_cast<Modio::ProfileMaturity::StorageType>(ProfileMaturityFlags);
	}
}
MODIODLL_EXPORT void SetModioModInfoMetadataBlob(CModioModInfo* Item, const CModioString* MetadataBlob)
{
	if (Item != nullptr)
	{
		Item->Impl.MetadataBlob = Modio::GetImpl(MetadataBlob);
	}
}
MODIODLL_EXPORT void SetModioModInfoFileInfo(CModioModInfo* Item, const CModioFileInfo* FileInfo)
{
	if (Item != nullptr)
	{
		if(FileInfo)
		{
			Item->Impl.FileInfo = Modio::GetImpl(FileInfo);
		}
		else if(Item->Impl.FileInfo.has_value())
		{
			Item->Impl.FileInfo.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioModInfoMetadataKVP(CModioModInfo* Item, const CModioMetadataKVP* MetadataKVP)
{
	if (Item != nullptr)
	{
		Item->Impl.MetadataKvp = Modio::GetImpl(MetadataKVP);
	}
}
MODIODLL_EXPORT void SetModioModInfoTags(CModioModInfo* Item, const CModioModTagList* Tags)
{
	if (Item != nullptr)
	{
		Item->Impl.Tags = Modio::GetImpl(Tags);
	}
}
MODIODLL_EXPORT void SetModioModInfoNumGalleryImages(CModioModInfo* Item, const size_t NumGalleryImages)
{
	if (Item != nullptr)
	{
		Item->Impl.NumGalleryImages = NumGalleryImages;
	}
}
MODIODLL_EXPORT void SetModioModInfoGallery(CModioModInfo* Item, const CModioImageList* Gallery)
{
	if (Item != nullptr)
	{
		Item->Impl.GalleryImages.GetRawList() = Modio::GetListImpl(Gallery);
	}
}
MODIODLL_EXPORT void SetModioModInfoStats(CModioModInfo* Item, const CModioModStats* Stats)
{
	if (Item != nullptr)
	{
		Item->Impl.Stats = Modio::GetImpl(Stats);
	}
}
MODIODLL_EXPORT void SetModioModInfoLogo(CModioModInfo* Item, const CModioModLogo* Logo)
{
	if (Item != nullptr)
	{
		Item->Impl.ModLogo = Modio::GetImpl(Logo);
	}
}
MODIODLL_EXPORT void SetModioModInfoVersion(CModioModInfo* Item, const CModioString* Version)
{
	if (Item != nullptr)
	{
		Item->Impl.Version = Modio::GetImpl(Version);
	}
}
MODIODLL_EXPORT void SetModioModInfoServerStatus(CModioModInfo* Item, const EModioModServerSideStatus ServerStatus)
{
	if (Item != nullptr)
	{
		Item->Impl.ModStatus = static_cast<Modio::ModServerSideStatus>(ServerStatus);
	}
}
MODIODLL_EXPORT void SetModioModInfoVisibility(CModioModInfo* Item, const EModioObjectVisibility Visibility)
{
	if (Item != nullptr)
	{
		Item->Impl.Visibility = static_cast<Modio::ObjectVisibility>(Visibility);
	}
}
MODIODLL_EXPORT void SetModioModInfoPrice(CModioModInfo* Item, const uint64_t Price)
{
	if (Item != nullptr)
	{
		Item->Impl.Price = Price;
	}
}
MODIODLL_EXPORT void SetModioModInfoDependencies(CModioModInfo* Item, const bool Dependencies)
{
	if (Item != nullptr)
	{
		Item->Impl.Dependencies = Dependencies;
	}
}
MODIODLL_EXPORT void SetModioModInfoCommunityOptions(CModioModInfo* Item, CModioModCommunityOptionsFlags CommunityOptions)
{
	if (Item != nullptr)
	{
		Item->Impl.CommunityOptions = static_cast<Modio::ModCommunityOptions>(CommunityOptions);
	}
}

MODIODLL_EXPORT CModioModCollectionInfo* CreateModioModCollectionInfo()
{
	return new CModioModCollectionInfo{ Modio::ModCollectionInfo {} };
}

MODIODLL_EXPORT void ReleaseModioModCollectionInfo(CModioModCollectionInfo* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModCollectionInfo* CopyModioModCollectionInfo(const CModioModCollectionInfo* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModCollectionInfo { *Item };
}

MODIODLL_EXPORT CModioModCollectionID* GetModioModCollectionInfoModCollectionID(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioModCollectionID{ ModCollectionInfo->Impl.Id };
}

MODIODLL_EXPORT CModioGameID* GetModioModCollectionInfoGameID(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioGameID{ ModCollectionInfo->Impl.GameId };
}

MODIODLL_EXPORT CModioString* GetModioModCollectionInfoGameName(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioString{ ModCollectionInfo->Impl.GameName };
}

MODIODLL_EXPORT EModioModServerSideStatus GetModioModCollectionInfoCollectionStatus(const CModioModCollectionInfo* ModCollectionInfo)
{
	return ConvertEnumToC( ModCollectionInfo->Impl.CollectionStatus );
}

MODIODLL_EXPORT EModioObjectVisibility GetModioModCollectionInfoVisibility(const CModioModCollectionInfo* ModCollectionInfo)
{
	return ConvertEnumToC( ModCollectionInfo->Impl.Visibility );
}

MODIODLL_EXPORT CModioUser* GetModioModCollectionInfoProfileSubmittedBy(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioUser{ ModCollectionInfo->Impl.ProfileSubmittedBy };
}

MODIODLL_EXPORT CModioString* GetModioModCollectionInfoCategory(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioString{ ModCollectionInfo->Impl.Category };
}

MODIODLL_EXPORT int64_t GetModioModCollectionInfoProfileDateAdded(const CModioModCollectionInfo* ModCollectionInfo)
{
	return int64_t{ ModCollectionInfo->Impl.ProfileDateAdded };
}

MODIODLL_EXPORT int64_t GetModioModCollectionInfoProfileDateUpdated(const CModioModCollectionInfo* ModCollectionInfo)
{
	return int64_t{ ModCollectionInfo->Impl.ProfileDateUpdated };
}

MODIODLL_EXPORT int64_t GetModioModCollectionInfoProfileDateLive(const CModioModCollectionInfo* ModCollectionInfo)
{
	return int64_t{ ModCollectionInfo->Impl.ProfileDateLive };
}

MODIODLL_EXPORT bool GetModioModCollectionInfoIncomplete(const CModioModCollectionInfo* ModCollectionInfo)
{
	return bool{ ModCollectionInfo->Impl.Incomplete };
}

MODIODLL_EXPORT CModioMaturityProfile GetModioModCollectionInfoProfileMaturityOption(const CModioModCollectionInfo* ModCollectionInfo)
{
	return CModioMaturityProfile(ModCollectionInfo->Impl.ProfileMaturityOption.RawValue());
}

MODIODLL_EXPORT uint64_t GetModioModCollectionInfoFileSize(const CModioModCollectionInfo* ModCollectionInfo)
{
	return uint64_t{ ModCollectionInfo->Impl.FileSize };
}

MODIODLL_EXPORT uint64_t GetModioModCollectionInfoFileSizeUncompressed(const CModioModCollectionInfo* ModCollectionInfo)
{
	return uint64_t{ ModCollectionInfo->Impl.FileSizeUncompressed };
}

MODIODLL_EXPORT CModioPlatformList* GetModioModCollectionInfoPlatforms(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioPlatformList{ ModCollectionInfo->Impl.Platforms };
}

MODIODLL_EXPORT CModioStringList* GetModioModCollectionInfoTags(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioStringList{ ModCollectionInfo->Impl.Tags };
}

MODIODLL_EXPORT CModioModCollectionStats* GetModioModCollectionInfoStats(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioModCollectionStats{ ModCollectionInfo->Impl.Stats };
}

MODIODLL_EXPORT CModioModLogo* GetModioModCollectionInfoLogo(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioModLogo{ ModCollectionInfo->Impl.Logo };
}

MODIODLL_EXPORT CModioString* GetModioModCollectionInfoProfileName(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioString{ ModCollectionInfo->Impl.ProfileName };
}

MODIODLL_EXPORT CModioString* GetModioModCollectionInfoProfileNameId(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioString{ ModCollectionInfo->Impl.ProfileNameId };
}

MODIODLL_EXPORT CModioString* GetModioModCollectionInfoProfileSummary(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioString{ ModCollectionInfo->Impl.ProfileSummary };
}

MODIODLL_EXPORT CModioString* GetModioModCollectionInfoProfileDescription(const CModioModCollectionInfo* ModCollectionInfo)
{
	return new CModioString{ ModCollectionInfo->Impl.ProfileDescription };
}

MODIODLL_EXPORT CModioString* GetModioModCollectionInfoProfileDescriptionPlaintext(const CModioModCollectionInfo* ModInfo)
{
	return new CModioString {ModInfo->Impl.ProfileDescriptionPlaintext};
}

MODIODLL_EXPORT void SetModioModCollectionInfoId(CModioModCollectionInfo* Item, const CModioModCollectionID* Id)
{
	if (Item != nullptr)
	{
		Item->Impl.Id = Modio::GetImpl(Id);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoGameId(CModioModCollectionInfo* Item, const CModioGameID* GameId)
{
	if (Item != nullptr)
	{
		Item->Impl.GameId = Modio::GetImpl(GameId);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoServerStatus(CModioModCollectionInfo* Item, const EModioModServerSideStatus ServerStatus)
{
	if (Item != nullptr)
	{
		Item->Impl.CollectionStatus = static_cast<Modio::ModServerSideStatus>(ServerStatus);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoVisibility(CModioModCollectionInfo* Item, const EModioObjectVisibility Visibility)
{
	if (Item != nullptr)
	{
		Item->Impl.Visibility = static_cast<Modio::ObjectVisibility>(Visibility);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoProfileSubmittedBy(CModioModCollectionInfo* Item, const CModioUser* ProfileSubmittedBy)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileSubmittedBy = Modio::GetImpl(ProfileSubmittedBy);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoCategory(CModioModCollectionInfo* Item, const CModioString* Category)
{
	if (Item != nullptr)
	{
		Item->Impl.Category = Modio::GetImpl(Category);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoProfileDateAdded(CModioModCollectionInfo* Item, const int64_t ProfileDateAdded)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDateAdded = ProfileDateAdded;
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoProfileDateUpdated(CModioModCollectionInfo* Item, const int64_t ProfileDateUpdated)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDateUpdated = ProfileDateUpdated;
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoProfileDateLive(CModioModCollectionInfo* Item, const int64_t ProfileDateLive)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDateLive = ProfileDateLive;
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoIncomplete(CModioModCollectionInfo* Item, const bool Incomplete)
{
	if (Item != nullptr)
	{
		Item->Impl.Incomplete = Incomplete;
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoProfileMaturityFlags(CModioModCollectionInfo* Item, CModioMaturityProfile ProfileMaturityFlags)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileMaturityOption = static_cast<Modio::ProfileMaturity::StorageType>(ProfileMaturityFlags);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoFileSize(CModioModCollectionInfo* Item, const uint64_t FileSize)
{
	if (Item != nullptr)
	{
		Item->Impl.FileSize = FileSize;
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoFileSizeUncompressed(CModioModCollectionInfo* Item, const uint64_t FileSizeUncompressed)
{
	if (Item != nullptr)
	{
		Item->Impl.FileSizeUncompressed = FileSizeUncompressed;
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoPlatforms(CModioModCollectionInfo* Item, const CModioPlatformList* Platforms)
{
	if (Item != nullptr)
	{
		Item->Impl.Platforms = Modio::GetImpl(Platforms);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoTags(CModioModCollectionInfo* Item, const CModioStringList* Tags)
{
	if (Item != nullptr)
	{
		Item->Impl.Tags = Modio::GetImpl(Tags);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoStats(CModioModCollectionInfo* Item, const CModioModCollectionStats* Stats)
{
	if (Item != nullptr)
	{
		Item->Impl.Stats = Modio::GetImpl(Stats);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoLogo(CModioModCollectionInfo* Item, const CModioModLogo* Logo)
{
	if (Item != nullptr)
	{
		Item->Impl.Logo = Modio::GetImpl(Logo);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoProfileName(CModioModCollectionInfo* Item, const CModioString* ProfileName)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileName = Modio::GetImpl(ProfileName);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoProfileNameId(CModioModCollectionInfo* Item, const CModioString* ProfileNameId)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileNameId = Modio::GetImpl(ProfileNameId);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoProfileSummary(CModioModCollectionInfo* Item, const CModioString* ProfileSummary)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileSummary = Modio::GetImpl(ProfileSummary);
	}
}
MODIODLL_EXPORT void SetModioModCollectionInfoProfileDescription(CModioModCollectionInfo* Item, const CModioString* ProfileDescription)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDescription = Modio::GetImpl(ProfileDescription);
	}
}

MODIODLL_EXPORT void SetModioModCollectionInfoProfileDescriptionPlaintext(CModioModCollectionInfo* Item,
																 const CModioString* ProfileDescriptionPlaintext)
{
	if (Item != nullptr)
	{
		Item->Impl.ProfileDescriptionPlaintext = Modio::GetImpl(ProfileDescriptionPlaintext);
	}
}

MODIODLL_EXPORT CModioImage* CreateModioImage()
{
	return new CModioImage{ Modio::Detail::Image {} };
}

MODIODLL_EXPORT void ReleaseModioImage(CModioImage* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioImage* CopyModioImage(const CModioImage* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioImage { *Item };
}

MODIODLL_EXPORT CModioString* GetModioImageFilename(const CModioImage* ModioImage)
{
	return new CModioString{ ModioImage->Impl.Filename };
}

MODIODLL_EXPORT CModioString* GetModioImageOriginalURL(const CModioImage* ModioImage)
{
	return new CModioString{ ModioImage->Impl.Original };
}

MODIODLL_EXPORT CModioString* GetModioImageThumb320x180(const CModioImage* ModioImage)
{
	return new CModioString{ ModioImage->Impl.Thumb320x180 };
}

MODIODLL_EXPORT CModioString* GetModioImageThumb1280x720(const CModioImage* ModioImage)
{
	return new CModioString{ ModioImage->Impl.Thumb1280x720 };
}


MODIODLL_EXPORT void SetModioImageFilename(CModioImage* Item, const CModioString* Filename)
{
	if (Item != nullptr)
	{
		Item->Impl.Filename = Modio::GetImpl(Filename);
	}
}
MODIODLL_EXPORT void SetModioImageOriginalURL(CModioImage* Item, const CModioString* OriginalURL)
{
	if (Item != nullptr)
	{
		Item->Impl.Original = Modio::GetImpl(OriginalURL);
	}
}
MODIODLL_EXPORT void SetModioImageThumb320x180(CModioImage* Item, const CModioString* Thumb320x180)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb320x180 = Modio::GetImpl(Thumb320x180);
	}
}
MODIODLL_EXPORT void SetModioImageThumb1280x720(CModioImage* Item, const CModioString* Thumb1280x720)
{
	if (Item != nullptr)
	{
		Item->Impl.Thumb1280x720 = Modio::GetImpl(Thumb1280x720);
	}
}

MODIODLL_EXPORT CModioCreateModCollectionParams* CreateModioCreateModCollectionParams()
{
	return new CModioCreateModCollectionParams{ Modio::CreateModCollectionParams {} };
}

MODIODLL_EXPORT void ReleaseModioCreateModCollectionParams(CModioCreateModCollectionParams* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioCreateModCollectionParams* CopyModioCreateModCollectionParams(const CModioCreateModCollectionParams* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioCreateModCollectionParams { *Item };
}

MODIODLL_EXPORT CModioString* GetModioCreateModCollectionParamsPathToLogoFile(const CModioCreateModCollectionParams* ModioCreateModCollectionParams)
{
	return new CModioString{ ModioCreateModCollectionParams->Impl.PathToLogoFile };
}

MODIODLL_EXPORT CModioString* GetModioCreateModCollectionParamsName(const CModioCreateModCollectionParams* ModioCreateModCollectionParams)
{
	return new CModioString{ ModioCreateModCollectionParams->Impl.Name };
}

MODIODLL_EXPORT CModioString* GetModioCreateModCollectionParamsSummary(const CModioCreateModCollectionParams* ModioCreateModCollectionParams)
{
	return new CModioString{ ModioCreateModCollectionParams->Impl.Summary };
}

MODIODLL_EXPORT CModioString* GetModioCreateModCollectionParamsCategory(const CModioCreateModCollectionParams* ModioCreateModCollectionParams)
{
	return new CModioString{ ModioCreateModCollectionParams->Impl.Category };
}

MODIODLL_EXPORT EModioObjectVisibility GetModioCreateModCollectionParamsVisibility(const CModioCreateModCollectionParams* ModioCreateModCollectionParams)
{
	return ConvertEnumToC( ModioCreateModCollectionParams->Impl.Visibility );
}

MODIODLL_EXPORT CModioModIDList* GetModioCreateModCollectionParamsMods(const CModioCreateModCollectionParams* ModioCreateModCollectionParams)
{
	if (ModioCreateModCollectionParams->Impl.Mods.has_value())
	{
		return new CModioModIDList{ *(ModioCreateModCollectionParams->Impl.Mods) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioCreateModCollectionParamsNamePath(const CModioCreateModCollectionParams* ModioCreateModCollectionParams)
{
	if (ModioCreateModCollectionParams->Impl.NamePath.has_value())
	{
		return new CModioString{ *(ModioCreateModCollectionParams->Impl.NamePath) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioCreateModCollectionParamsDescription(const CModioCreateModCollectionParams* ModioCreateModCollectionParams)
{
	if (ModioCreateModCollectionParams->Impl.Description.has_value())
	{
		return new CModioString{ *(ModioCreateModCollectionParams->Impl.Description) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioStringList* GetModioCreateModCollectionParamsTags(const CModioCreateModCollectionParams* ModioCreateModCollectionParams)
{
	if (ModioCreateModCollectionParams->Impl.Tags.has_value())
	{
		return new CModioStringList{ *(ModioCreateModCollectionParams->Impl.Tags) };
	}
	else
	{
		return nullptr;
	}
}


MODIODLL_EXPORT void SetModioCreateModCollectionParamsPathToLogoFile(CModioCreateModCollectionParams* Item, const CModioString* PathToLogoFile)
{
	if (Item != nullptr)
	{
		Item->Impl.PathToLogoFile = Modio::GetImpl(PathToLogoFile);
	}
}
MODIODLL_EXPORT void SetModioCreateModCollectionParamsName(CModioCreateModCollectionParams* Item, const CModioString* Name)
{
	if (Item != nullptr)
	{
		Item->Impl.Name = Modio::GetImpl(Name);
	}
}
MODIODLL_EXPORT void SetModioCreateModCollectionParamsSummary(CModioCreateModCollectionParams* Item, const CModioString* Summary)
{
	if (Item != nullptr)
	{
		Item->Impl.Summary = Modio::GetImpl(Summary);
	}
}
MODIODLL_EXPORT void SetModioCreateModCollectionParamsCategory(CModioCreateModCollectionParams* Item, const CModioString* Category)
{
	if (Item != nullptr)
	{
		Item->Impl.Category = Modio::GetImpl(Category);
	}
}
MODIODLL_EXPORT void SetModioCreateModCollectionParamsVisibility(CModioCreateModCollectionParams* Item, const EModioObjectVisibility Visibility)
{
	if (Item != nullptr)
	{
		Item->Impl.Visibility = static_cast<Modio::ObjectVisibility>(Visibility);
	}
}
MODIODLL_EXPORT void SetModioCreateModCollectionParamsMods(CModioCreateModCollectionParams* Item, const CModioModIDList* Mods)
{
	if (Item != nullptr)
	{
		if(Mods)
		{
			Item->Impl.Mods = Modio::GetImpl(Mods);
		}
		else if(Item->Impl.Mods.has_value())
		{
			Item->Impl.Mods.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModCollectionParamsNamePath(CModioCreateModCollectionParams* Item, const CModioString* NamePath)
{
	if (Item != nullptr)
	{
		if(NamePath)
		{
			Item->Impl.NamePath = Modio::GetImpl(NamePath);
		}
		else if(Item->Impl.NamePath.has_value())
		{
			Item->Impl.NamePath.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModCollectionParamsDescription(CModioCreateModCollectionParams* Item, const CModioString* Description)
{
	if (Item != nullptr)
	{
		if(Description)
		{
			Item->Impl.Description = Modio::GetImpl(Description);
		}
		else if(Item->Impl.Description.has_value())
		{
			Item->Impl.Description.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModCollectionParamsTags(CModioCreateModCollectionParams* Item, const CModioStringList* Tags)
{
	if (Item != nullptr)
	{
		if(Tags)
		{
			Item->Impl.Tags = Modio::GetImpl(Tags);
		}
		else if(Item->Impl.Tags.has_value())
		{
			Item->Impl.Tags.reset();
		}
	}
}

MODIODLL_EXPORT CModioCreateModParams* CreateModioCreateModParams()
{
	return new CModioCreateModParams{ Modio::CreateModParams {} };
}

MODIODLL_EXPORT void ReleaseModioCreateModParams(CModioCreateModParams* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioCreateModParams* CopyModioCreateModParams(const CModioCreateModParams* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioCreateModParams { *Item };
}

MODIODLL_EXPORT CModioString* GetModioCreateModParamsPathToLogoFile(const CModioCreateModParams* ModioCreateModParams)
{
	return new CModioString{ ModioCreateModParams->Impl.PathToLogoFile };
}

MODIODLL_EXPORT CModioString* GetModioCreateModParamsName(const CModioCreateModParams* ModioCreateModParams)
{
	return new CModioString{ ModioCreateModParams->Impl.Name };
}

MODIODLL_EXPORT CModioString* GetModioCreateModParamsSummary(const CModioCreateModParams* ModioCreateModParams)
{
	return new CModioString{ ModioCreateModParams->Impl.Summary };
}

MODIODLL_EXPORT CModioString* GetModioCreateModParamsNamePath(const CModioCreateModParams* ModioCreateModParams)
{
	if (ModioCreateModParams->Impl.NamePath.has_value())
	{
		return new CModioString{ *(ModioCreateModParams->Impl.NamePath) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT COptionalObjectVisibility GetModioCreateModParamsVisibility(const CModioCreateModParams* ModioCreateModParams)
{
	if (ModioCreateModParams->Impl.Visibility.has_value())
	{
		return { ConvertEnumToC( *(ModioCreateModParams->Impl.Visibility) ), true };
	}
	else
	{
		return { ConvertEnumToC(Modio::ObjectVisibility::Hidden), false };
	}
}

MODIODLL_EXPORT CModioString* GetModioCreateModParamsDescription(const CModioCreateModParams* ModioCreateModParams)
{
	if (ModioCreateModParams->Impl.Description.has_value())
	{
		return new CModioString{ *(ModioCreateModParams->Impl.Description) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioCreateModParamsHomepageURL(const CModioCreateModParams* ModioCreateModParams)
{
	if (ModioCreateModParams->Impl.HomepageURL.has_value())
	{
		return new CModioString{ *(ModioCreateModParams->Impl.HomepageURL) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT COptionalUInt64 GetModioCreateModParamsStock(const CModioCreateModParams* ModioCreateModParams)
{
	if (ModioCreateModParams->Impl.Stock.has_value())
	{
		return COptionalUInt64{ *(ModioCreateModParams->Impl.Stock), true };
;
	}
	else
	{
		return { 0, false };
	}
}

MODIODLL_EXPORT COptionalModioMaturityProfile GetModioCreateModParamsMaturityRating(const CModioCreateModParams* ModioCreateModParams)
{
	if (ModioCreateModParams->Impl.MaturityRating.has_value())
	{
		return { ConvertEnumToC(static_cast<Modio::MaturityOption>(*(ModioCreateModParams->Impl.MaturityRating))), true };
	}
	else
	{
		return { ConvertEnumToC(Modio::MaturityOption::None), false };
	}
}

MODIODLL_EXPORT CModioString* GetModioCreateModParamsMetadataBlob(const CModioCreateModParams* ModioCreateModParams)
{
	if (ModioCreateModParams->Impl.MetadataBlob.has_value())
	{
		return new CModioString{ *(ModioCreateModParams->Impl.MetadataBlob) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioStringList* GetModioCreateModParamsTags(const CModioCreateModParams* ModioCreateModParams)
{
	if (ModioCreateModParams->Impl.Tags.has_value())
	{
		return new CModioStringList{ *(ModioCreateModParams->Impl.Tags) };
	}
	else
	{
		return nullptr;
	}
}


MODIODLL_EXPORT void SetModioCreateModParamsPathToLogoFile(CModioCreateModParams* Item, const CModioString* PathToLogoFile)
{
	if (Item != nullptr)
	{
		Item->Impl.PathToLogoFile = Modio::GetImpl(PathToLogoFile);
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsName(CModioCreateModParams* Item, const CModioString* Name)
{
	if (Item != nullptr)
	{
		Item->Impl.Name = Modio::GetImpl(Name);
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsSummary(CModioCreateModParams* Item, const CModioString* Summary)
{
	if (Item != nullptr)
	{
		Item->Impl.Summary = Modio::GetImpl(Summary);
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsNamePath(CModioCreateModParams* Item, const CModioString* NamePath)
{
	if (Item != nullptr)
	{
		if(NamePath)
		{
			Item->Impl.NamePath = Modio::GetImpl(NamePath);
		}
		else if(Item->Impl.NamePath.has_value())
		{
			Item->Impl.NamePath.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsVisibility(CModioCreateModParams* Item, const EModioObjectVisibility* Visibility)
{
	if (Item != nullptr)
	{
		if(Visibility)
		{
			Item->Impl.Visibility = static_cast<Modio::ObjectVisibility>(*Visibility);
		}
		else if(Item->Impl.Visibility.has_value())
		{
			Item->Impl.Visibility.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsDescription(CModioCreateModParams* Item, const CModioString* Description)
{
	if (Item != nullptr)
	{
		if(Description)
		{
			Item->Impl.Description = Modio::GetImpl(Description);
		}
		else if(Item->Impl.Description.has_value())
		{
			Item->Impl.Description.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsHomepageURL(CModioCreateModParams* Item, const CModioString* HomepageURL)
{
	if (Item != nullptr)
	{
		if(HomepageURL)
		{
			Item->Impl.HomepageURL = Modio::GetImpl(HomepageURL);
		}
		else if(Item->Impl.HomepageURL.has_value())
		{
			Item->Impl.HomepageURL.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsStock(CModioCreateModParams* Item, const size_t* Stock)
{
	if (Item != nullptr)
	{
		if(Stock)
		{
			Item->Impl.Stock = *Stock;
		}
		else if(Item->Impl.Stock.has_value())
		{
			Item->Impl.Stock.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsMaturityRating(CModioCreateModParams* Item, CModioMaturityProfile const* MaturityRating)
{
	if (Item != nullptr)
	{
		if(MaturityRating)
		{
			Item->Impl.MaturityRating = static_cast<Modio::MaturityOption>(*MaturityRating);
		}
		else if(Item->Impl.MaturityRating.has_value())
		{
			Item->Impl.MaturityRating.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsMetadataBlob(CModioCreateModParams* Item, const CModioString* MetadataBlob)
{
	if (Item != nullptr)
	{
		if(MetadataBlob)
		{
			Item->Impl.MetadataBlob = Modio::GetImpl(MetadataBlob);
		}
		else if(Item->Impl.MetadataBlob.has_value())
		{
			Item->Impl.MetadataBlob.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModParamsTags(CModioCreateModParams* Item, const CModioStringList* Tags)
{
	if (Item != nullptr)
	{
		if(Tags)
		{
			Item->Impl.Tags = Modio::GetImpl(Tags);
		}
		else if(Item->Impl.Tags.has_value())
		{
			Item->Impl.Tags.reset();
		}
	}
}

MODIODLL_EXPORT CModioPlatformList* CreateModioPlatformList()
{
	return new CModioPlatformList{ std::vector<Modio::ModfilePlatform> {} };
}

MODIODLL_EXPORT void ReleaseModioPlatformList(CModioPlatformList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioPlatformList* CopyModioPlatformList(const CModioPlatformList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioPlatformList { *Item };
}


MODIODLL_EXPORT EModioModfilePlatform GetModioPlatformListPlatformByIndex(const CModioPlatformList* ModioPlatformList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioPlatformList).size())
	{
		return EModioModfilePlatform( Modio::GetImpl(ModioPlatformList)[Index] );
	}
	else
	{	
		return EModioModfilePlatform{};
	}	
}
MODIODLL_EXPORT void SetModioPlatformListPlatformByIndex(CModioPlatformList* ModioPlatformList, uint64_t Index, const EModioModfilePlatform  Value)
{
	if (Index < Modio::GetImpl(ModioPlatformList).size())
	{
			Modio::GetImpl(ModioPlatformList)[Index] = std::vector<Modio::ModfilePlatform>::value_type(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioPlatformListCount(const CModioPlatformList* ModioPlatformList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioPlatformList).size());
}

MODIODLL_EXPORT void SetModioPlatformListCount(CModioPlatformList* ModioPlatformList, uint64_t Count)
{
	Modio::GetImpl(ModioPlatformList).resize(Count);
}


MODIODLL_EXPORT CModioGamePlatformList* CreateModioGamePlatformList()
{
	return new CModioGamePlatformList{ std::vector<Modio::GamePlatform> {} };
}

MODIODLL_EXPORT void ReleaseModioGamePlatformList(CModioGamePlatformList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioGamePlatformList* CopyModioGamePlatformList(const CModioGamePlatformList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioGamePlatformList { *Item };
}


MODIODLL_EXPORT CModioGamePlatform* GetModioGamePlatformListPlatformByIndex(const CModioGamePlatformList* ModioGamePlatformList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioGamePlatformList).size())
	{
		return new CModioGamePlatform{ Modio::GetImpl(ModioGamePlatformList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioGamePlatformListPlatformByIndex(CModioGamePlatformList* ModioGamePlatformList, uint64_t Index, const CModioGamePlatform * Value)
{
	if (Index < Modio::GetImpl(ModioGamePlatformList).size())
	{
			Modio::GetImpl(ModioGamePlatformList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioGamePlatformListCount(const CModioGamePlatformList* ModioGamePlatformList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioGamePlatformList).size());
}

MODIODLL_EXPORT void SetModioGamePlatformListCount(CModioGamePlatformList* ModioGamePlatformList, uint64_t Count)
{
	Modio::GetImpl(ModioGamePlatformList).resize(Count);
}


MODIODLL_EXPORT CModioCreateModFileParams* CreateModioCreateModFileParams(const char* RootDirectoryPath, size_t RootDirectoryLength)
{
return new CModioCreateModFileParams{ Modio::CreateModFileParams { std::string { RootDirectoryPath, RootDirectoryLength } } };
}

MODIODLL_EXPORT void ReleaseModioCreateModFileParams(CModioCreateModFileParams* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioCreateModFileParams* CopyModioCreateModFileParams(const CModioCreateModFileParams* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioCreateModFileParams { *Item };
}

MODIODLL_EXPORT CModioString* GetModioCreateModFileParamsRootDirectory(const CModioCreateModFileParams* CreateParams)
{
	return new CModioString{ CreateParams->Impl.RootDirectory };
}

MODIODLL_EXPORT CModioString* GetModioCreateModFileParamsVersion(const CModioCreateModFileParams* CreateParams)
{
	if (CreateParams->Impl.Version.has_value())
	{
		return new CModioString{ *(CreateParams->Impl.Version) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioString* GetModioCreateModFileParamsChangelog(const CModioCreateModFileParams* CreateParams)
{
	if (CreateParams->Impl.Changelog.has_value())
	{
		return new CModioString{ *(CreateParams->Impl.Changelog) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT COptionalBool GetModioCreateModFileParamsSetAsActive(const CModioCreateModFileParams* CreateParams)
{
	if (CreateParams->Impl.bSetAsActive.has_value())
	{
		return COptionalBool{ *(CreateParams->Impl.bSetAsActive), true };
;
	}
	else
	{
		return { false, false };
	}
}

MODIODLL_EXPORT CModioString* GetModioCreateModFileParamsMetadataBlob(const CModioCreateModFileParams* CreateParams)
{
	if (CreateParams->Impl.MetadataBlob.has_value())
	{
		return new CModioString{ *(CreateParams->Impl.MetadataBlob) };
	}
	else
	{
		return nullptr;
	}
}

MODIODLL_EXPORT CModioPlatformList* GetModioCreateModFileParamsPlatforms(const CModioCreateModFileParams* CreateParams)
{
	if (CreateParams->Impl.Platforms.has_value())
	{
		return new CModioPlatformList{ *(CreateParams->Impl.Platforms) };
	}
	else
	{
		return nullptr;
	}
}


MODIODLL_EXPORT void SetModioCreateModFileParamsRootDirectory(CModioCreateModFileParams* Item, const CModioString* RootDirectory)
{
	if (Item != nullptr)
	{
		Item->Impl.RootDirectory = Modio::GetImpl(RootDirectory);
	}
}
MODIODLL_EXPORT void SetModioCreateModFileParamsVersion(CModioCreateModFileParams* Item, const CModioString* Version)
{
	if (Item != nullptr)
	{
		if(Version)
		{
			Item->Impl.Version = Modio::GetImpl(Version);
		}
		else if(Item->Impl.Version.has_value())
		{
			Item->Impl.Version.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModFileParamsChangelog(CModioCreateModFileParams* Item, const CModioString* Changelog)
{
	if (Item != nullptr)
	{
		if(Changelog)
		{
			Item->Impl.Changelog = Modio::GetImpl(Changelog);
		}
		else if(Item->Impl.Changelog.has_value())
		{
			Item->Impl.Changelog.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModFileParamsSetAsActive(CModioCreateModFileParams* Item, const bool* SetAsActive)
{
	if (Item != nullptr)
	{
		if(SetAsActive)
		{
			Item->Impl.bSetAsActive = *SetAsActive;
		}
		else if(Item->Impl.bSetAsActive.has_value())
		{
			Item->Impl.bSetAsActive.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModFileParamsMetadataBlob(CModioCreateModFileParams* Item, const CModioString* MetadataBlob)
{
	if (Item != nullptr)
	{
		if(MetadataBlob)
		{
			Item->Impl.MetadataBlob = Modio::GetImpl(MetadataBlob);
		}
		else if(Item->Impl.MetadataBlob.has_value())
		{
			Item->Impl.MetadataBlob.reset();
		}
	}
}
MODIODLL_EXPORT void SetModioCreateModFileParamsPlatforms(CModioCreateModFileParams* Item, const CModioPlatformList* Platforms)
{
	if (Item != nullptr)
	{
		if(Platforms)
		{
			Item->Impl.Platforms = Modio::GetImpl(Platforms);
		}
		else if(Item->Impl.Platforms.has_value())
		{
			Item->Impl.Platforms.reset();
		}
	}
}

MODIODLL_EXPORT CModioAuthToken* CreateModioAuthToken()
{
	return new CModioAuthToken{ Modio::Detail::OAuthToken {} };
}

MODIODLL_EXPORT void ReleaseModioAuthToken(CModioAuthToken* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioAuthToken* CopyModioAuthToken(const CModioAuthToken* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioAuthToken { *Item };
}

MODIODLL_EXPORT EAuthTokenState GetModioAuthTokenState(const CModioAuthToken* ModioAuthToken)
{
	return ConvertEnumToC( ModioAuthToken->Impl.GetTokenState() );
}

MODIODLL_EXPORT CModioString* GetModioAuthTokenRawToken(const CModioAuthToken* ModioAuthToken)
{
	if (ModioAuthToken->Impl.GetToken().has_value())
	{
		return new CModioString{ *(ModioAuthToken->Impl.GetToken()) };
	}
	else
	{
		return nullptr;
	}
}



MODIODLL_EXPORT CModioModID* CreateModioModID(int64_t ID)
{
return new CModioModID{ Modio::ModID { ID } };
}

MODIODLL_EXPORT void ReleaseModioModID(CModioModID* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModID* CopyModioModID(const CModioModID* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModID { *Item };
}

MODIODLL_EXPORT bool AreEqualModioModID(const CModioModID* First, const CModioModID* Second)
{
	return Modio::GetImpl(First) == Modio::GetImpl(Second);
}


MODIODLL_EXPORT bool ModioModIDIsValid(const CModioModID* Item)
{
	return Modio::GetImpl(Item).IsValid();
}

MODIODLL_EXPORT int64_t ModioModIDAsInteger(const CModioModID* Item)
{
	return Modio::GetImpl(Item);
}


MODIODLL_EXPORT CModioModCollectionID* CreateModioModCollectionID(int64_t ID)
{
return new CModioModCollectionID{ Modio::ModCollectionID { ID } };
}

MODIODLL_EXPORT void ReleaseModioModCollectionID(CModioModCollectionID* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModCollectionID* CopyModioModCollectionID(const CModioModCollectionID* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModCollectionID { *Item };
}

MODIODLL_EXPORT bool AreEqualModioModCollectionID(const CModioModCollectionID* First, const CModioModCollectionID* Second)
{
	return Modio::GetImpl(First) == Modio::GetImpl(Second);
}


MODIODLL_EXPORT bool ModioModCollectionIDIsValid(const CModioModCollectionID* Item)
{
	return Modio::GetImpl(Item).IsValid();
}

MODIODLL_EXPORT int64_t ModioModCollectionIDAsInteger(const CModioModCollectionID* Item)
{
	return Modio::GetImpl(Item);
}


MODIODLL_EXPORT CModioModIDList* CreateModioModIDList()
{
	return new CModioModIDList{ std::vector<Modio::ModID> {} };
}

MODIODLL_EXPORT void ReleaseModioModIDList(CModioModIDList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioModIDList* CopyModioModIDList(const CModioModIDList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioModIDList { *Item };
}


MODIODLL_EXPORT CModioModID* GetModioModIDListModByIndex(const CModioModIDList* ModIDList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModIDList).size())
	{
		return new CModioModID{ Modio::GetImpl(ModIDList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioModIDListModByIndex(CModioModIDList* ModIDList, uint64_t Index, const CModioModID * Value)
{
	if (Index < Modio::GetImpl(ModIDList).size())
	{
			Modio::GetImpl(ModIDList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioModIDListCount(const CModioModIDList* ModIDList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModIDList).size());
}

MODIODLL_EXPORT void SetModioModIDListCount(CModioModIDList* ModIDList, uint64_t Count)
{
	Modio::GetImpl(ModIDList).resize(Count);
}


MODIODLL_EXPORT CModioUserID* CreateModioUserID(int64_t ID)
{
return new CModioUserID{ Modio::UserID { ID } };
}

MODIODLL_EXPORT void ReleaseModioUserID(CModioUserID* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioUserID* CopyModioUserID(const CModioUserID* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioUserID { *Item };
}

MODIODLL_EXPORT bool AreEqualModioUserID(const CModioUserID* First, const CModioUserID* Second)
{
	return Modio::GetImpl(First) == Modio::GetImpl(Second);
}


MODIODLL_EXPORT bool ModioUserIDIsValid(const CModioUserID* Item)
{
	return Modio::GetImpl(Item).IsValid();
}

MODIODLL_EXPORT int64_t ModioUserIDAsInteger(const CModioUserID* Item)
{
	return Modio::GetImpl(Item);
}


MODIODLL_EXPORT CModioUserIDList* CreateModioUserIDList()
{
	return new CModioUserIDList{ std::vector<Modio::UserID> {} };
}

MODIODLL_EXPORT void ReleaseModioUserIDList(CModioUserIDList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioUserIDList* CopyModioUserIDList(const CModioUserIDList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioUserIDList { *Item };
}


MODIODLL_EXPORT CModioUserID* GetModioUserIDListUserByIndex(const CModioUserIDList* UserList, uint64_t Index)
{
	if (Index < Modio::GetImpl(UserList).size())
	{
		return new CModioUserID{ Modio::GetImpl(UserList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioUserIDListUserByIndex(CModioUserIDList* UserList, uint64_t Index, const CModioUserID * Value)
{
	if (Index < Modio::GetImpl(UserList).size())
	{
			Modio::GetImpl(UserList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioUserIDListCount(const CModioUserIDList* UserList)
{
	return static_cast<uint64_t>(Modio::GetImpl(UserList).size());
}

MODIODLL_EXPORT void SetModioUserIDListCount(CModioUserIDList* UserList, uint64_t Count)
{
	Modio::GetImpl(UserList).resize(Count);
}


MODIODLL_EXPORT CModioFileMetadataID* CreateModioFileMetadataID(int64_t ID)
{
return new CModioFileMetadataID{ Modio::FileMetadataID { ID } };
}

MODIODLL_EXPORT void ReleaseModioFileMetadataID(CModioFileMetadataID* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioFileMetadataID* CopyModioFileMetadataID(const CModioFileMetadataID* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioFileMetadataID { *Item };
}

MODIODLL_EXPORT bool AreEqualModioFileMetadataID(const CModioFileMetadataID* First, const CModioFileMetadataID* Second)
{
	return Modio::GetImpl(First) == Modio::GetImpl(Second);
}



MODIODLL_EXPORT CModioInitializeOptions* CreateModioInitializeOptions()
{
	return new CModioInitializeOptions{ Modio::InitializeOptions {} };
}

MODIODLL_EXPORT void ReleaseModioInitializeOptions(CModioInitializeOptions* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioInitializeOptions* CopyModioInitializeOptions(const CModioInitializeOptions* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioInitializeOptions { *Item };
}

MODIODLL_EXPORT CModioGameID* GetModioInitializeOptionsGameID(const CModioInitializeOptions* InitializeOptions)
{
	return new CModioGameID{ InitializeOptions->Impl.GameID };
}

MODIODLL_EXPORT CModioAPIKey* GetModioInitializeOptionsAPIKey(const CModioInitializeOptions* InitializeOptions)
{
	return new CModioAPIKey{ InitializeOptions->Impl.APIKey };
}

MODIODLL_EXPORT CModioString* GetModioInitializeOptionsUserSession(const CModioInitializeOptions* InitializeOptions)
{
	return new CModioString{ InitializeOptions->Impl.User };
}

MODIODLL_EXPORT EModioPortal GetModioInitializeOptionsPortal(const CModioInitializeOptions* InitializeOptions)
{
	return ConvertEnumToC( InitializeOptions->Impl.PortalInUse );
}

MODIODLL_EXPORT EModioEnvironment GetModioInitializeOptionsEnvironment(const CModioInitializeOptions* InitializeOptions)
{
	return ConvertEnumToC( InitializeOptions->Impl.GameEnvironment );
}

MODIODLL_EXPORT CModioStringMap* GetModioInitializeOptionsExtendedParameters(const CModioInitializeOptions* InitializeOptions)
{
	return new CModioStringMap{ InitializeOptions->Impl.ExtendedParameters };
}


MODIODLL_EXPORT void SetModioInitializeOptionsGameID(CModioInitializeOptions* Item, const CModioGameID* GameID)
{
	if (Item != nullptr)
	{
		Item->Impl.GameID = Modio::GetImpl(GameID);
	}
}
MODIODLL_EXPORT void SetModioInitializeOptionsAPIKey(CModioInitializeOptions* Item, const CModioAPIKey* APIKey)
{
	if (Item != nullptr)
	{
		Item->Impl.APIKey = Modio::GetImpl(APIKey);
	}
}
MODIODLL_EXPORT void SetModioInitializeOptionsUserSession(CModioInitializeOptions* Item, const CModioString* UserSession)
{
	if (Item != nullptr)
	{
		Item->Impl.User = Modio::GetImpl(UserSession);
	}
}
MODIODLL_EXPORT void SetModioInitializeOptionsPortal(CModioInitializeOptions* Item, const EModioPortal Portal)
{
	if (Item != nullptr)
	{
		Item->Impl.PortalInUse = static_cast<Modio::Portal>(Portal);
	}
}
MODIODLL_EXPORT void SetModioInitializeOptionsEnvironment(CModioInitializeOptions* Item, const EModioEnvironment Environment)
{
	if (Item != nullptr)
	{
		Item->Impl.GameEnvironment = static_cast<Modio::Environment>(Environment);
	}
}
MODIODLL_EXPORT void SetModioInitializeOptionsExtendedParameters(CModioInitializeOptions* Item, const CModioStringMap* ExtendedParameters)
{
	if (Item != nullptr)
	{
		Item->Impl.ExtendedParameters = Modio::GetImpl(ExtendedParameters);
	}
}

MODIODLL_EXPORT CModioGameID* CreateModioGameID(int64_t ID)
{
return new CModioGameID{ Modio::GameID { ID } };
}

MODIODLL_EXPORT void ReleaseModioGameID(CModioGameID* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioGameID* CopyModioGameID(const CModioGameID* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioGameID { *Item };
}

MODIODLL_EXPORT bool AreEqualModioGameID(const CModioGameID* First, const CModioGameID* Second)
{
	return Modio::GetImpl(First) == Modio::GetImpl(Second);
}


MODIODLL_EXPORT bool ModioGameIDIsValid(const CModioGameID* Item)
{
	return Modio::GetImpl(Item).IsValid();
}

MODIODLL_EXPORT int64_t ModioGameIDAsInteger(const CModioGameID* Item)
{
	return Modio::GetImpl(Item);
}


MODIODLL_EXPORT void ReleaseModioReportParams(CModioReportParams* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioReportParams* CopyModioReportParams(const CModioReportParams* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioReportParams { *Item };
}



MODIODLL_EXPORT CModioTransactionRecord* CreateModioTransactionRecord()
{
	return new CModioTransactionRecord{ Modio::TransactionRecord {} };
}

MODIODLL_EXPORT void ReleaseModioTransactionRecord(CModioTransactionRecord* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioTransactionRecord* CopyModioTransactionRecord(const CModioTransactionRecord* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioTransactionRecord { *Item };
}

MODIODLL_EXPORT CModioModID* GetModioTransactionRecordAssociatedMod(const CModioTransactionRecord* ModioTransactionRecord)
{
	return new CModioModID{ ModioTransactionRecord->Impl.AssociatedModID };
}

MODIODLL_EXPORT uint64_t GetModioTransactionRecordPrice(const CModioTransactionRecord* ModioTransactionRecord)
{
	return uint64_t{ ModioTransactionRecord->Impl.Price };
}

MODIODLL_EXPORT uint64_t GetModioTransactionRecordUpdatedUserWalletBalance(const CModioTransactionRecord* ModioTransactionRecord)
{
	return uint64_t{ ModioTransactionRecord->Impl.UpdatedUserWalletBalance };
}

MODIODLL_EXPORT CModioModInfo* GetModioTransactionRecordMod(const CModioTransactionRecord* ModioTransactionRecord)
{
	return new CModioModInfo{ ModioTransactionRecord->Impl.Mod };
}


MODIODLL_EXPORT void SetModioTransactionRecordAssociatedMod(CModioTransactionRecord* Item, const CModioModID* AssociatedMod)
{
	if (Item != nullptr)
	{
		Item->Impl.AssociatedModID = Modio::GetImpl(AssociatedMod);
	}
}
MODIODLL_EXPORT void SetModioTransactionRecordPrice(CModioTransactionRecord* Item, const uint64_t Price)
{
	if (Item != nullptr)
	{
		Item->Impl.Price = Price;
	}
}
MODIODLL_EXPORT void SetModioTransactionRecordUpdatedUserWalletBalance(CModioTransactionRecord* Item, const uint64_t UpdatedUserWalletBalance)
{
	if (Item != nullptr)
	{
		Item->Impl.UpdatedUserWalletBalance = UpdatedUserWalletBalance;
	}
}
MODIODLL_EXPORT void SetModioTransactionRecordMod(CModioTransactionRecord* Item, const CModioModInfo* Mod)
{
	if (Item != nullptr)
	{
		Item->Impl.Mod = Modio::GetImpl(Mod);
	}
}

MODIODLL_EXPORT CModioEntitlementParams* CreateModioEntitlementParams()
{
	return new CModioEntitlementParams{ Modio::EntitlementParams {} };
}

MODIODLL_EXPORT void ReleaseModioEntitlementParams(CModioEntitlementParams* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioEntitlementParams* CopyModioEntitlementParams(const CModioEntitlementParams* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioEntitlementParams { *Item };
}

MODIODLL_EXPORT CModioStringMap* GetModioEntitlementParamsExtendedParameters(const CModioEntitlementParams* EntitlementParams)
{
	return new CModioStringMap{ EntitlementParams->Impl.ExtendedParameters };
}


MODIODLL_EXPORT void SetModioEntitlementParamsExtendedParameters(CModioEntitlementParams* Item, const CModioStringMap* ExtendedParameters)
{
	if (Item != nullptr)
	{
		Item->Impl.ExtendedParameters = Modio::GetImpl(ExtendedParameters);
	}
}

MODIODLL_EXPORT CModioChangeMap* CreateModioChangeMap()
{
	return new CModioChangeMap{ std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType> {} };
}

MODIODLL_EXPORT void ReleaseModioChangeMap(CModioChangeMap* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioChangeMap* CopyModioChangeMap(const CModioChangeMap* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioChangeMap { *Item };
}


MODIODLL_EXPORT EModioChangeType ModioChangeMapGetKeyValue(CModioChangeMap* Item, CModioModID const* Key)
{
	if(Modio::GetImpl(Item).count(Modio::GetImpl(Key)))
	{
		return ConvertEnumToC(Modio::GetImpl(Item)[Modio::GetImpl(Key)]);
	}
	return {};
}

MODIODLL_EXPORT CModioChangeMapIterator* ModioChangeMapBegin(CModioChangeMap* Item)
{ 
	if(Modio::GetImpl(Item).begin() == Modio::GetImpl(Item).end())
		return nullptr;
	return new CModioChangeMapIterator{ Modio::GetImpl(Item).begin(), Modio::GetImpl(Item).end() };
}

MODIODLL_EXPORT void ReleaseModioChangeMapIterator(CModioChangeMapIterator* Item)
{
	delete Item;
}

MODIODLL_EXPORT bool ModioChangeMapNext(CModioChangeMapIterator* Iterator)
{
	return (++Iterator->Iter) != Iterator->End;
}

MODIODLL_EXPORT CModioModID* ModioChangeMapGetKey(CModioChangeMapIterator* Iterator)
{
	return new CModioModID{ Iterator->Iter->first };
}

MODIODLL_EXPORT EModioChangeType ModioChangeMapGetValue(CModioChangeMapIterator* Iterator)
{
	return ConvertEnumToC(Iterator->Iter->second);
}

MODIODLL_EXPORT void ModioChangeMapSetKeyValue(CModioChangeMap* Item, CModioModID const *Key, EModioChangeType Value)
{
	Modio::GetImpl(Item)[Modio::GetImpl(Key)] = static_cast<Modio::UserSubscriptionList::ChangeType>(Value);
}


MODIODLL_EXPORT CModioEntitlementConsumptionStatus* CreateModioEntitlementConsumptionStatus()
{
	return new CModioEntitlementConsumptionStatus{ Modio::EntitlementConsumptionStatus {} };
}

MODIODLL_EXPORT void ReleaseModioEntitlementConsumptionStatus(CModioEntitlementConsumptionStatus* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioEntitlementConsumptionStatus* CopyModioEntitlementConsumptionStatus(const CModioEntitlementConsumptionStatus* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioEntitlementConsumptionStatus { *Item };
}

MODIODLL_EXPORT CModioString* GetModioEntitlementConsumptionStatusTransactionId(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus)
{
	return new CModioString{ EntitlementConsumptionStatus->Impl.TransactionId };
}

MODIODLL_EXPORT EModioEntitlementConsumptionState GetModioEntitlementConsumptionStatusTransactionState(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus)
{
	return ConvertEnumToC( EntitlementConsumptionStatus->Impl.TransactionState );
}

MODIODLL_EXPORT CModioString* GetModioEntitlementConsumptionStatusSkuId(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus)
{
	return new CModioString{ EntitlementConsumptionStatus->Impl.SkuId };
}

MODIODLL_EXPORT bool GetModioEntitlementConsumptionStatusEntitlementConsumed(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus)
{
	return bool{ EntitlementConsumptionStatus->Impl.EntitlementConsumed };
}

MODIODLL_EXPORT EModioEntitlementType GetModioEntitlementConsumptionStatusEntitlementType(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus)
{
	return ConvertEnumToC( EntitlementConsumptionStatus->Impl.EntitlementType );
}

MODIODLL_EXPORT int32_t GetModioEntitlementConsumptionStatusTokensAllocated(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus)
{
	return int32_t{ EntitlementConsumptionStatus->Impl.VirtualCurrencyDetails.TokensAllocated };
}

MODIODLL_EXPORT bool GetModioEntitlementConsumptionStatusEntitlementRequiresRetry(const CModioEntitlementConsumptionStatus* EntitlementConsumptionStatus)
{
	return bool{ EntitlementConsumptionStatus->Impl.EntitlementRequiresRetry() };
}


MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusTransactionId(CModioEntitlementConsumptionStatus* Item, const CModioString* TransactionId)
{
	if (Item != nullptr)
	{
		Item->Impl.TransactionId = Modio::GetImpl(TransactionId);
	}
}
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusTransactionState(CModioEntitlementConsumptionStatus* Item, const EModioEntitlementConsumptionState TransactionState)
{
	if (Item != nullptr)
	{
		Item->Impl.TransactionState = static_cast<Modio::EntitlementConsumptionState>(TransactionState);
	}
}
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusSkuId(CModioEntitlementConsumptionStatus* Item, const CModioString* SkuId)
{
	if (Item != nullptr)
	{
		Item->Impl.SkuId = Modio::GetImpl(SkuId);
	}
}
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusEntitlementConsumed(CModioEntitlementConsumptionStatus* Item, const bool EntitlementConsumed)
{
	if (Item != nullptr)
	{
		Item->Impl.EntitlementConsumed = EntitlementConsumed;
	}
}
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusEntitlementType(CModioEntitlementConsumptionStatus* Item, const EModioEntitlementType EntitlementType)
{
	if (Item != nullptr)
	{
		Item->Impl.EntitlementType = static_cast<Modio::EntitlementType>(EntitlementType);
	}
}
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusTokensAllocated(CModioEntitlementConsumptionStatus* Item, const int32_t TokensAllocated)
{
	if (Item != nullptr)
	{
		Item->Impl.VirtualCurrencyDetails.TokensAllocated = TokensAllocated;
	}
}

MODIODLL_EXPORT CModioEntitlementConsumptionStatusList* CreateModioEntitlementConsumptionStatusList()
{
	return new CModioEntitlementConsumptionStatusList{ Modio::EntitlementConsumptionStatusList {} };
}

MODIODLL_EXPORT void ReleaseModioEntitlementConsumptionStatusList(CModioEntitlementConsumptionStatusList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioEntitlementConsumptionStatusList* CopyModioEntitlementConsumptionStatusList(const CModioEntitlementConsumptionStatusList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioEntitlementConsumptionStatusList { *Item };
}

MODIODLL_EXPORT CModioEntitlementConsumptionStatusList* GetModioEntitlementConsumptionStatusListEntitlementsThatRequireRetry(const CModioEntitlementConsumptionStatusList* EntitlementConsumptionStatus)
{
	if (EntitlementConsumptionStatus->Impl.EntitlementsThatRequireRetry().has_value())
	{
		return new CModioEntitlementConsumptionStatusList{ *(EntitlementConsumptionStatus->Impl.EntitlementsThatRequireRetry()) };
	}
	else
	{
		return nullptr;
	}
}


MODIODLL_EXPORT CModioEntitlementConsumptionStatus* GetModioEntitlementConsumptionStatusListEntitlementByIndex(const CModioEntitlementConsumptionStatusList* ModioEntitlementList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioEntitlementList).Size())
	{
		return new CModioEntitlementConsumptionStatus{ Modio::GetImpl(ModioEntitlementList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusListEntitlementByIndex(CModioEntitlementConsumptionStatusList* ModioEntitlementList, uint64_t Index, const CModioEntitlementConsumptionStatus * Value)
{
	if (Index < Modio::GetImpl(ModioEntitlementList).Size())
	{
			Modio::GetImpl(ModioEntitlementList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioEntitlementConsumptionStatusListCount(const CModioEntitlementConsumptionStatusList* ModioEntitlementList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioEntitlementList).Size());
}

MODIODLL_EXPORT void SetModioEntitlementConsumptionStatusListCount(CModioEntitlementConsumptionStatusList* ModioEntitlementList, uint64_t Count)
{
	Modio::GetImpl(ModioEntitlementList).GetRawList().resize(Count);
}


MODIODLL_EXPORT CModioEntitlementList* CreateModioEntitlementList()
{
	return new CModioEntitlementList{ Modio::EntitlementList {} };
}

MODIODLL_EXPORT void ReleaseModioEntitlementList(CModioEntitlementList* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioEntitlementList* CopyModioEntitlementList(const CModioEntitlementList* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioEntitlementList { *Item };
}


MODIODLL_EXPORT CModioEntitlement* GetModioEntitlementListAvailableEntitlementByIndex(const CModioEntitlementList* ModioEntitlementList, uint64_t Index)
{
	if (Index < Modio::GetImpl(ModioEntitlementList).Size())
	{
		return new CModioEntitlement{ Modio::GetImpl(ModioEntitlementList)[Index] };
	}
	else
	{	
		return nullptr;
	}	
}
MODIODLL_EXPORT void SetModioEntitlementListAvailableEntitlementByIndex(CModioEntitlementList* ModioEntitlementList, uint64_t Index, const CModioEntitlement * Value)
{
	if (Index < Modio::GetImpl(ModioEntitlementList).Size())
	{
			Modio::GetImpl(ModioEntitlementList)[Index] = Modio::GetImpl(Value);
		}
}

MODIODLL_EXPORT uint64_t GetModioEntitlementListCount(const CModioEntitlementList* ModioEntitlementList)
{
	return static_cast<uint64_t>(Modio::GetImpl(ModioEntitlementList).Size());
}

MODIODLL_EXPORT void SetModioEntitlementListCount(CModioEntitlementList* ModioEntitlementList, uint64_t Count)
{
	Modio::GetImpl(ModioEntitlementList).GetRawList().resize(Count);
}


MODIODLL_EXPORT CModioEntitlement* CreateModioEntitlement()
{
	return new CModioEntitlement{ Modio::Entitlement {} };
}

MODIODLL_EXPORT void ReleaseModioEntitlement(CModioEntitlement* Item)
{
	Modio::CModioSafeDelete(Item);
}

MODIODLL_EXPORT CModioEntitlement* CopyModioEntitlement(const CModioEntitlement* Item)
{
	if(Item == nullptr)
	{
		return nullptr;
	}
	return new CModioEntitlement { *Item };
}

MODIODLL_EXPORT CModioString* GetModioEntitlementSkuId(const CModioEntitlement* Entitlement)
{
	return new CModioString{ Entitlement->Impl.SkuId };
}

MODIODLL_EXPORT EModioEntitlementType GetModioEntitlementEntitlementType(const CModioEntitlement* Entitlement)
{
	return ConvertEnumToC( Entitlement->Impl.Type );
}





MODIODLL_EXPORT CModioErrorCode* ModioInitTempModSet(CModioModIDList* ModIds)
{
	return new CModioErrorCode{ Modio::InitTempModSet(Modio::GetImpl<CModioModIDList>(ModIds))};
}	


MODIODLL_EXPORT CModioErrorCode* ModioAddToTempModSet(CModioModIDList* ModIds)
{
	return new CModioErrorCode{ Modio::AddToTempModSet(Modio::GetImpl<CModioModIDList>(ModIds))};
}	


MODIODLL_EXPORT CModioErrorCode* ModioRemoveFromTempModSet(CModioModIDList* ModIds)
{
	return new CModioErrorCode{ Modio::RemoveFromTempModSet(Modio::GetImpl<CModioModIDList>(ModIds))};
}	


MODIODLL_EXPORT CModioErrorCode* ModioCloseTempModSet()
{
	return new CModioErrorCode{ Modio::CloseTempModSet()};
}	


MODIODLL_EXPORT CModioModCollectionMap* ModioQueryTempModSet()
{
	return new CModioModCollectionMap{ Modio::QueryTempModSet()};
}	


MODIODLL_EXPORT void ModioInitializeAsync(CModioInitializeOptions* InitOptions, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, InitOptions))
	{
		Modio::InitializeAsync(Modio::GetImpl<CModioInitializeOptions>(InitOptions), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioSetLogLevel(EModioLogLevel Level)
{
	Modio::SetLogLevel(static_cast<Modio::LogLevel>(Level));
}	


MODIODLL_EXPORT void ModioListModCollectionsAsync(CModioFilterParams* FilterParams, ModioModCollectionListCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::ModCollectionInfoList>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::ModCollectionInfoList> InTerms) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModCollectionInfoList Terms { *InTerms };
			Callback(&ErrorCode, &Terms, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, FilterParams))
	{
		Modio::ListModCollectionsAsync(Modio::GetImpl<CModioFilterParams>(FilterParams), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioListUserFollowedModCollectionsAsync(CModioFilterParams* FilterParams, ModioModCollectionListCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::ModCollectionInfoList>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::ModCollectionInfoList> InTerms) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModCollectionInfoList Terms { *InTerms };
			Callback(&ErrorCode, &Terms, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, FilterParams))
	{
		Modio::ListUserFollowedModCollectionsAsync(Modio::GetImpl<CModioFilterParams>(FilterParams), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetModCollectionInfoAsync(CModioModCollectionID* ModCollectionID, ModioModCollectionInfoCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::ModCollectionInfo>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::ModCollectionInfo> InTerms) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModCollectionInfo Terms { *InTerms };
			Callback(&ErrorCode, &Terms, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModCollectionID))
	{
		Modio::GetModCollectionInfoAsync(Modio::GetImpl<CModioModCollectionID>(ModCollectionID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetModCollectionModsAsync(CModioModCollectionID* ModCollectionID, ModioModListCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::ModInfoList>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::ModInfoList> InTerms) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModInfoList Terms { *InTerms };
			Callback(&ErrorCode, &Terms, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModCollectionID))
	{
		Modio::GetModCollectionModsAsync(Modio::GetImpl<CModioModCollectionID>(ModCollectionID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioSubmitModCollectionRatingAsync(CModioModCollectionID* ModCollectionID, EModioRating Rating, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModCollectionID, Rating))
	{
		Modio::SubmitModCollectionRatingAsync(Modio::GetImpl<CModioModCollectionID>(ModCollectionID), static_cast<Modio::Rating>(Rating), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioSubscribeToModCollectionAsync(CModioModCollectionID* ModCollectionID, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModCollectionID))
	{
		Modio::SubscribeToModCollectionAsync(Modio::GetImpl<CModioModCollectionID>(ModCollectionID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioUnsubscribeFromModCollectionAsync(CModioModCollectionID* ModCollectionID, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModCollectionID))
	{
		Modio::UnsubscribeFromModCollectionAsync(Modio::GetImpl<CModioModCollectionID>(ModCollectionID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioFollowModCollectionAsync(CModioModCollectionID* ModCollectionID, ModioModCollectionInfoCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::ModCollectionInfo>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::ModCollectionInfo> InTerms) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModCollectionInfo Terms { *InTerms };
			Callback(&ErrorCode, &Terms, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModCollectionID))
	{
		Modio::FollowModCollectionAsync(Modio::GetImpl<CModioModCollectionID>(ModCollectionID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioUnfollowModCollectionAsync(CModioModCollectionID* ModCollectionID, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModCollectionID))
	{
		Modio::UnfollowModCollectionAsync(Modio::GetImpl<CModioModCollectionID>(ModCollectionID), WrappedCallback);
	}
}	


MODIODLL_EXPORT CModioModCreationHandle* ModioGetModCreationHandle()
{
	return new CModioModCreationHandle{ Modio::GetModCreationHandle()};
}	


MODIODLL_EXPORT void ModioSubscribeToModAsync(CModioModID* ModID, bool IncludeDependencies, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, IncludeDependencies))
	{
		Modio::SubscribeToModAsync(Modio::GetImpl<CModioModID>(ModID), (IncludeDependencies), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioSubmitNewModAsync(CModioModCreationHandle* Handle, CModioCreateModParams* Params, ModioSubmitNewModCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModID>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, Modio::Optional<Modio::ModID> OptionalModID) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModID ModID { *OptionalModID };
			Callback(&ErrorCode, &ModID, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, Handle, Params))
	{
		Modio::SubmitNewModAsync(Modio::GetImpl<CModioModCreationHandle>(Handle), Modio::GetImpl<CModioCreateModParams>(Params), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioUnsubscribeFromModAsync(CModioModID* ModID, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID))
	{
		Modio::UnsubscribeFromModAsync(Modio::GetImpl<CModioModID>(ModID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioFetchExternalUpdatesAsync(ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::FetchExternalUpdatesAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioPreviewExternalUpdatesAsync(ModioPreviewExternalUpdatesCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, std::map<Modio::ModID, Modio::UserSubscriptionList::ChangeType> OnPreviewDone) {
		CModioErrorCode ErrorCode { ec };
		CModioChangeMap PreviewDone { OnPreviewDone };
		
		if (!ec)
		{
			Callback(&ErrorCode, &PreviewDone, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::PreviewExternalUpdatesAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT CModioErrorCode* ModioEnableModManagement(ModioModManagementCallback Callback, void* ContextPtr)
{
    std::function<void(Modio::ModManagementEvent)> WrappedCallback = 
        [Callback, ContextPtr](Modio::ModManagementEvent InEvent) {
            CModioModManagementEvent Event { InEvent };
            
            Callback(&Event, ContextPtr
            );

        };
    return new CModioErrorCode{ Modio::EnableModManagement(WrappedCallback)};
}	


MODIODLL_EXPORT void ModioDisableModManagement()
{
	Modio::DisableModManagement();
}	


MODIODLL_EXPORT bool ModioIsModManagementBusy()
{
	return bool{ Modio::IsModManagementBusy()};
}	


MODIODLL_EXPORT CModioErrorCode* ModioPrioritizeTransferForMod(CModioModID* ModID)
{
	return new CModioErrorCode{ Modio::PrioritizeTransferForMod(Modio::GetImpl<CModioModID>(ModID))};
}	


MODIODLL_EXPORT CModioModProgressInfo* ModioQueryCurrentModUpdate()
{
	auto OptionalReturn = Modio::QueryCurrentModUpdate();
	if (OptionalReturn.has_value())
	{
		return new CModioModProgressInfo{ *(OptionalReturn) };
	}
	else
	{
		return nullptr;
	}
}	


MODIODLL_EXPORT void ModioSetLogCallback(ModioLogCallback Callback, void* ContextPtr)
{
    std::function<void(Modio::LogLevel, std::string)> WrappedCallback = 
        [Callback, ContextPtr](Modio::LogLevel InLogLevel, std::string InMessage) {
            EModioLogLevel LogLevel { ConvertEnumToC( InLogLevel ) };
            CModioString Message { InMessage };
            
            Callback(LogLevel, &Message, ContextPtr
            );

        };
    Modio::SetLogCallback(WrappedCallback);
}	


MODIODLL_EXPORT void ModioRunPendingHandlers()
{
	Modio::RunPendingHandlers();
}	


MODIODLL_EXPORT CModioModCollectionMap* ModioQueryUserSubscriptions()
{
	return new CModioModCollectionMap{ Modio::QueryUserSubscriptions()};
}	


MODIODLL_EXPORT CModioModCollectionMap* ModioQueryUserInstallations(bool bIncludeOutdatedMods)
{
	return new CModioModCollectionMap{ Modio::QueryUserInstallations((bIncludeOutdatedMods))};
}	


MODIODLL_EXPORT CModioModCollectionMap* ModioQuerySystemInstallations()
{
	return new CModioModCollectionMap{ Modio::QuerySystemInstallations()};
}	


MODIODLL_EXPORT void ModioSubmitModRatingAsync(CModioModID* ModID, EModioRating Rating, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, Rating))
	{
		Modio::SubmitModRatingAsync(Modio::GetImpl<CModioModID>(ModID), static_cast<Modio::Rating>(Rating), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetModTagOptionsAsync(ModioModTagOptionsCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::ModTagOptions>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::ModTagOptions> InOptions) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModTagOptions Options { *InOptions };
			Callback(&ErrorCode, &Options, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::GetModTagOptionsAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT CModioUser* ModioQueryUserProfile()
{
	auto OptionalReturn = Modio::QueryUserProfile();
	if (OptionalReturn.has_value())
	{
		return new CModioUser{ *(OptionalReturn) };
	}
	else
	{
		return nullptr;
	}
}	


MODIODLL_EXPORT void ModioSetLanguage(EModioLanguage Locale)
{
	Modio::SetLanguage(static_cast<Modio::Language>(Locale));
}	


MODIODLL_EXPORT void ModioShutdownAsync(ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::ShutdownAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetModInfoAsync(CModioModID* ModID, ModioGetModInfoCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, Modio::Optional<Modio::ModInfo> OptionalModInfo) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModInfo ModInfo { *OptionalModInfo };
			Callback(&ErrorCode, &ModInfo, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID))
	{
		Modio::GetModInfoAsync(Modio::GetImpl<CModioModID>(ModID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioForceUninstallModAsync(CModioModID* ModToRemove, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModToRemove))
	{
		Modio::ForceUninstallModAsync(Modio::GetImpl<CModioModID>(ModToRemove), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioAuthenticateUserExternalAsync(CModioAuthenticationParams* User, EModioAuthenticationProvider Provider, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, User, Provider))
	{
		Modio::AuthenticateUserExternalAsync(Modio::GetImpl<CModioAuthenticationParams>(User), static_cast<Modio::AuthenticationProvider>(Provider), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioVerifyUserAuthenticationAsync(ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::VerifyUserAuthenticationAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioRefreshUserDataAsync(ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::RefreshUserDataAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetModCreatorAvatarAsync(CModioModID* ModID, EModioAvatarSize AvatarSize, ModioGetModMediaCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, Modio::Optional<std::string> OptionalPath) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioString Path { *OptionalPath };
			Callback(&ErrorCode, &Path, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, AvatarSize))
	{
		Modio::GetModMediaAsync(Modio::GetImpl<CModioModID>(ModID), static_cast<Modio::AvatarSize>(AvatarSize), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetModLogoAsync(CModioModID* ModID, EModioLogoSize LogoSize, ModioGetModMediaCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, Modio::Optional<std::string> OptionalPath) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioString Path { *OptionalPath };
			Callback(&ErrorCode, &Path, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, LogoSize))
	{
		Modio::GetModMediaAsync(Modio::GetImpl<CModioModID>(ModID), static_cast<Modio::LogoSize>(LogoSize), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetModGalleryImageAsync(CModioModID* ModID, EModioGallerySize GallerySize, uint32_t Index, ModioGetModMediaCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, Modio::Optional<std::string> OptionalPath) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioString Path { *OptionalPath };
			Callback(&ErrorCode, &Path, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, GallerySize, Index))
	{
		Modio::GetModMediaAsync(Modio::GetImpl<CModioModID>(ModID), static_cast<Modio::GallerySize>(GallerySize), static_cast<size_t>(Index), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetTermsOfUseAsync(ModioTermsOfUseCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::Terms>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::Terms> InTerms) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioTermsOfUse Terms { *InTerms };
			Callback(&ErrorCode, &Terms, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::GetTermsOfUseAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioSubmitModChangesAsync(CModioModID* ModID, CModioEditModParams* Params, ModioSubmitModChangesCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, Modio::Optional<Modio::ModInfo> OptionalModInfo) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModInfo ModInfo { *OptionalModInfo };
			Callback(&ErrorCode, &ModInfo, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, Params))
	{
		Modio::SubmitModChangesAsync(Modio::GetImpl<CModioModID>(ModID), Modio::GetImpl<CModioEditModParams>(Params), WrappedCallback);
	}
}	


MODIODLL_EXPORT bool ModioErrorCodeMatches(CModioErrorCode* ec, EModioErrorCondition Condition)
{
	return bool{ Modio::ErrorCodeMatches(Modio::GetImpl<CModioErrorCode>(ec), static_cast<Modio::ErrorConditionTypes>(Condition))};
}	


MODIODLL_EXPORT void ModioListAllModsAsync(CModioFilterParams* Filter, ModioModListCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::ModInfoList>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::ModInfoList> InTerms) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModInfoList Terms { *InTerms };
			Callback(&ErrorCode, &Terms, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, Filter))
	{
		Modio::ListAllModsAsync(Modio::GetImpl<CModioFilterParams>(Filter), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioListUserCreatedModsAsync(CModioFilterParams* Filter, ModioModListCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::ModInfoList>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::ModInfoList> InTerms) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModInfoList Terms { *InTerms };
			Callback(&ErrorCode, &Terms, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, Filter))
	{
		Modio::ListUserCreatedModsAsync(Modio::GetImpl<CModioFilterParams>(Filter), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetModDependenciesAsync(CModioModID* ModID, bool Recursive, ModioModDependenciesCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::ModDependencyList>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::ModDependencyList> InDependencies) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioModDependencyList Dependencies { *InDependencies };
			Callback(&ErrorCode, &Dependencies, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, Recursive))
	{
		Modio::GetModDependenciesAsync(Modio::GetImpl<CModioModID>(ModID), (Recursive), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioAddModDependenciesAsync(CModioModID* ModID, CModioModIDList* Dependencies, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, Dependencies))
	{
		Modio::AddModDependenciesAsync(Modio::GetImpl<CModioModID>(ModID), Modio::GetImpl<CModioModIDList>(Dependencies), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioDeleteModDependenciesAsync(CModioModID* ModID, CModioModIDList* Dependencies, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, Dependencies))
	{
		Modio::DeleteModDependenciesAsync(Modio::GetImpl<CModioModID>(ModID), Modio::GetImpl<CModioModIDList>(Dependencies), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioRequestEmailAuthCodeAsync(CModioString* EmailAddress, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, EmailAddress))
	{
		Modio::RequestEmailAuthCodeAsync(Modio::ConstructFromImpl<Modio::EmailAddress>(EmailAddress), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioAuthenticateUserEmailAsync(CModioString* EmailAddress, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, EmailAddress))
	{
		Modio::AuthenticateUserEmailAsync(Modio::ConstructFromImpl<Modio::EmailAuthCode>(EmailAddress), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioClearUserDataAsync(ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::ClearUserDataAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetUserMediaAsync(EModioAvatarSize AvatarSize, ModioGetModMediaCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, Modio::Optional<std::string> OptionalPath) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioString Path { *OptionalPath };
			Callback(&ErrorCode, &Path, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, AvatarSize))
	{
		Modio::GetUserMediaAsync(static_cast<Modio::AvatarSize>(AvatarSize), WrappedCallback);
	}
}	


MODIODLL_EXPORT CModioValidationErrorList* ModioGetLastValidationError()
{
	return new CModioValidationErrorList{ Modio::GetLastValidationError()};
}	


MODIODLL_EXPORT CModioReportParams* ModioCreateModioReportParamsForGame(CModioGameID* GameID, EModioReportType ReportType, CModioString* ReportDescription, CModioString* ReporterName, CModioString* ReporterContact)
{
	return new CModioReportParams{ Modio::ReportParams(Modio::GetImpl<CModioGameID>(GameID), static_cast<Modio::ReportType>(ReportType), Modio::GetImpl<CModioString>(ReportDescription), Modio::GetImpl<CModioString>(ReporterName), Modio::GetImpl<CModioString>(ReporterContact))};
}	


MODIODLL_EXPORT CModioReportParams* ModioCreateModioReportParamsForUser(CModioUserID* UserID, EModioReportType ReportType, CModioString* ReportDescription, CModioString* ReporterName, CModioString* ReporterContact)
{
	return new CModioReportParams{ Modio::ReportParams(Modio::GetImpl<CModioUserID>(UserID), static_cast<Modio::ReportType>(ReportType), Modio::GetImpl<CModioString>(ReportDescription), Modio::GetImpl<CModioString>(ReporterName), Modio::GetImpl<CModioString>(ReporterContact))};
}	


MODIODLL_EXPORT CModioReportParams* ModioCreateModioReportParamsForMod(CModioModID* ModID, EModioReportType ReportType, CModioString* ReportDescription, CModioString* ReporterName, CModioString* ReporterContact)
{
	return new CModioReportParams{ Modio::ReportParams(Modio::GetImpl<CModioModID>(ModID), static_cast<Modio::ReportType>(ReportType), Modio::GetImpl<CModioString>(ReportDescription), Modio::GetImpl<CModioString>(ReporterName), Modio::GetImpl<CModioString>(ReporterContact))};
}	


MODIODLL_EXPORT CModioStringList* ModioGetBaseModInstallationDirectories()
{
	return new CModioStringList{ Modio::GetBaseModInstallationDirectories()};
}	


MODIODLL_EXPORT void ModioReportContentAsync(CModioReportParams* ReportParams, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ReportParams))
	{
		Modio::ReportContentAsync(Modio::GetImpl<CModioReportParams>(ReportParams), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioAddOrUpdateModLogoAsync(CModioModID* ModID, CModioString* NewLogoPath, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, NewLogoPath))
	{
		Modio::AddOrUpdateModLogoAsync(Modio::GetImpl<CModioModID>(ModID), Modio::GetImpl<CModioString>(NewLogoPath), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioArchiveModAsync(CModioModID* ModID, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID))
	{
		Modio::ArchiveModAsync(Modio::GetImpl<CModioModID>(ModID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioMuteUserAsync(CModioUserID* UserID, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, UserID))
	{
		Modio::MuteUserAsync(Modio::GetImpl<CModioUserID>(UserID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioUnmuteUserAsync(CModioUserID* UserID, ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, UserID))
	{
		Modio::UnmuteUserAsync(Modio::GetImpl<CModioUserID>(UserID), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioGetMutedUsersAsync(ModioUserListCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::UserList>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::UserList> InUserList) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioUserList UserList { *InUserList };
			Callback(&ErrorCode, &UserList, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::GetMutedUsersAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioPurchaseModAsync(CModioModID* ModID, uint64_t ExpectedPrice, ModioTransactionCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::TransactionRecord>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::TransactionRecord> InTransactionRecord) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioTransactionRecord TransactionRecord { *InTransactionRecord };
			Callback(&ErrorCode, &TransactionRecord, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, ExpectedPrice))
	{
		Modio::PurchaseModAsync(Modio::GetImpl<CModioModID>(ModID), (ExpectedPrice), WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioPurchaseModWithEntitlementAsync(CModioModID* ModID, CModioEntitlementParams* Params, ModioTransactionCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<Modio::TransactionRecord>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<Modio::TransactionRecord> InTransactionRecord) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioTransactionRecord TransactionRecord { *InTransactionRecord };
			Callback(&ErrorCode, &TransactionRecord, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, ModID, Params))
	{
		Modio::PurchaseModWithEntitlementAsync(Modio::GetImpl<CModioModID>(ModID), Modio::GetImpl<CModioEntitlementParams>(Params), WrappedCallback);
	}
}	


MODIODLL_EXPORT CModioString* ModioGetDefaultModInstallationDirectory(CModioGameID* GameID)
{
	return new CModioString{ Modio::GetDefaultModInstallationDirectory(Modio::GetImpl<CModioGameID>(GameID))};
}	


MODIODLL_EXPORT void ModioGetUserWalletBalanceAsync(ModioWalletBalanceCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, tl::optional<uint64_t>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, tl::optional<uint64_t> InBalance) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			uint64_t Balance { *InBalance };
			Callback(&ErrorCode, &Balance, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::GetUserWalletBalanceAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioFetchUserPurchasesAsync(ModioErrorCodeCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec) {
		CModioErrorCode ErrorCode { ec };
		
		if (!ec)
		{
			Callback(&ErrorCode, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback))
	{
		Modio::FetchUserPurchasesAsync(WrappedCallback);
	}
}	


MODIODLL_EXPORT void ModioRefreshUserEntitlementsAsync(CModioEntitlementParams* Params, ModioUserEntitlementCallback Callback, void* ContextPtr)
{
	std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)> WrappedCallback = 
	[Callback, ContextPtr](Modio::ErrorCode ec, Modio::Optional<Modio::EntitlementConsumptionStatusList> EntitlementList) {
		CModioErrorCode ErrorCode { ec };
				
		if (!ec)
		{
			CModioEntitlementConsumptionStatusList CEntitlementList { *EntitlementList };
			Callback(&ErrorCode, &CEntitlementList, ContextPtr);
		}
		else
		{
			Callback(&ErrorCode, nullptr, ContextPtr);
		}
	};
	if (RequireParametersNotNull(WrappedCallback, Params))
	{
		Modio::RefreshUserEntitlementsAsync(Modio::GetImpl<CModioEntitlementParams>(Params), WrappedCallback);
	}
}	



