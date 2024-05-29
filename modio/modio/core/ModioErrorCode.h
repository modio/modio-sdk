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

#include <system_error>

#include "modio/core/ModioSplitCompilation.h"
#include "modio/core/ModioStdTypes.h" //For ErrorCode type alias

namespace Modio
{
	enum class HttpError
	{
		CannotOpenConnection = 20481,
		DownloadNotPermitted = 20482,
		ExcessiveRedirects = 20483,
		HttpAlreadyInitialized = 20484,
		HttpNotInitialized = 20485,
		InsufficientPermissions = 20486,
		InvalidResponse = 20487,
		RateLimited = 20488,
		RequestError = 20489,
		ResourceNotAvailable = 20490,
		SecurityConfigurationInvalid = 20491,
		ServerClosedConnection = 20492,
		ServerUnavailable = 20493,
		ServersOverloaded = 20494
	};

	/// @docnone
	struct HttpErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::HttpError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::HttpError>(ErrorValue))
			{
				case HttpError::CannotOpenConnection:
						return "Unable to connect to server";
					break;
				case HttpError::DownloadNotPermitted:
						return "Trying to download file from outside of mod.io domain";
					break;
				case HttpError::ExcessiveRedirects:
						return "Exceeded the allowed number of redirects";
					break;
				case HttpError::HttpAlreadyInitialized:
						return "HTTP service already initialized";
					break;
				case HttpError::HttpNotInitialized:
						return "HTTP service not initialized";
					break;
				case HttpError::InsufficientPermissions:
						return "Insufficient permissions";
					break;
				case HttpError::InvalidResponse:
						return "The HTTP response was malformed or not in the expected format";
					break;
				case HttpError::RateLimited:
						return "Too many requests made to the mod.io API within the rate-limiting window. Please wait and try again";
					break;
				case HttpError::RequestError:
						return "An error occurred making a HTTP request";
					break;
				case HttpError::ResourceNotAvailable:
						return "Invalid endpoint path";
					break;
				case HttpError::SecurityConfigurationInvalid:
						return "Invalid platform HTTP security configuration";
					break;
				case HttpError::ServerClosedConnection:
						return "Server closed connection unexpectedly";
					break;
				case HttpError::ServerUnavailable:
						return "Unable to connect to server";
					break;
				case HttpError::ServersOverloaded:
						return "The mod.io servers are overloaded. Please wait a bit before trying again";
					break;
				default:
					return "Unknown HttpError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& HttpErrorCategory()
	{
		static HttpErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(HttpError e)
	{
		return { static_cast<int>(e), HttpErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, HttpError B)
	{
		return A.category() == HttpErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, HttpError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, HttpError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class FilesystemError
	{
		DirectoryNotEmpty = 20737,
		DirectoryNotFound = 20738,
		FileLocked = 20739,
		FileNotFound = 20740,
		InsufficientSpace = 20741,
		NoPermission = 20742,
		ReadError = 20743,
		UnableToCreateFile = 20744,
		UnableToCreateFolder = 20745,
		WriteError = 20746
	};

	/// @docnone
	struct FilesystemErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::FilesystemError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::FilesystemError>(ErrorValue))
			{
				case FilesystemError::DirectoryNotEmpty:
						return "Directory not empty";
					break;
				case FilesystemError::DirectoryNotFound:
						return "Directory not found";
					break;
				case FilesystemError::FileLocked:
						return "File locked (already in use?)";
					break;
				case FilesystemError::FileNotFound:
						return "File not found";
					break;
				case FilesystemError::InsufficientSpace:
						return "Insufficient space for file";
					break;
				case FilesystemError::NoPermission:
						return "Insufficient permission for filesystem operation";
					break;
				case FilesystemError::ReadError:
						return "Error reading file";
					break;
				case FilesystemError::UnableToCreateFile:
						return "Could not create file";
					break;
				case FilesystemError::UnableToCreateFolder:
						return "Could not create folder";
					break;
				case FilesystemError::WriteError:
						return "Error writing file";
					break;
				default:
					return "Unknown FilesystemError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& FilesystemErrorCategory()
	{
		static FilesystemErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(FilesystemError e)
	{
		return { static_cast<int>(e), FilesystemErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, FilesystemError B)
	{
		return A.category() == FilesystemErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, FilesystemError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, FilesystemError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class UserAuthError
	{
		AlreadyAuthenticated = 20993,
		EmailLoginCodeExpired = 20994,
		EmailLoginCodeInvalid = 20995,
		NoAuthToken = 20996,
		StatusAuthTokenInvalid = 20997,
		StatusAuthTokenMissing = 20998,
		UnableToInitStorage = 20999
	};

	/// @docnone
	struct UserAuthErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::UserAuthError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::UserAuthError>(ErrorValue))
			{
				case UserAuthError::AlreadyAuthenticated:
						return "User is already authenticated. To use a new user and OAuth token, call ClearUserDataAsync";
					break;
				case UserAuthError::EmailLoginCodeExpired:
						return "Email login code has expired, please request a new one.";
					break;
				case UserAuthError::EmailLoginCodeInvalid:
						return "Email login code is invalid";
					break;
				case UserAuthError::NoAuthToken:
						return "No Auth token available";
					break;
				case UserAuthError::StatusAuthTokenInvalid:
						return "The user's OAuth token was invalid";
					break;
				case UserAuthError::StatusAuthTokenMissing:
						return "OAuth token was missing";
					break;
				case UserAuthError::UnableToInitStorage:
						return "Could not initialize user storage";
					break;
				default:
					return "Unknown UserAuthError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& UserAuthErrorCategory()
	{
		static UserAuthErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(UserAuthError e)
	{
		return { static_cast<int>(e), UserAuthErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, UserAuthError B)
	{
		return A.category() == UserAuthErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, UserAuthError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, UserAuthError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class UserDataError
	{
		BlobMissing = 21249,
		InvalidUser = 21250
	};

	/// @docnone
	struct UserDataErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::UserDataError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::UserDataError>(ErrorValue))
			{
				case UserDataError::BlobMissing:
						return "Some or all of the user data was missing from storage";
					break;
				case UserDataError::InvalidUser:
						return "Invalid user";
					break;
				default:
					return "Unknown UserDataError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& UserDataErrorCategory()
	{
		static UserDataErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(UserDataError e)
	{
		return { static_cast<int>(e), UserDataErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, UserDataError B)
	{
		return A.category() == UserDataErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, UserDataError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, UserDataError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class ArchiveError
	{
		InvalidHeader = 21505,
		UnsupportedCompression = 21506
	};

	/// @docnone
	struct ArchiveErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::ArchiveError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::ArchiveError>(ErrorValue))
			{
				case ArchiveError::InvalidHeader:
						return "File did not have a valid archive header";
					break;
				case ArchiveError::UnsupportedCompression:
						return "File uses an unsupported compression method. Please use STORE or DEFLATE";
					break;
				default:
					return "Unknown ArchiveError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& ArchiveErrorCategory()
	{
		static ArchiveErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(ArchiveError e)
	{
		return { static_cast<int>(e), ArchiveErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, ArchiveError B)
	{
		return A.category() == ArchiveErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, ArchiveError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, ArchiveError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class GenericError
	{
		BadParameter = 21761,
		CouldNotCreateHandle = 21762,
		EndOfFile = 21763,
		IndexOutOfRange = 21764,
		NoDataAvailable = 21765,
		OperationCanceled = 21766,
		OperationError = 21767,
		QueueClosed = 21768,
		SDKAlreadyInitialized = 21769,
		SDKNotInitialized = 21770
	};

	/// @docnone
	struct GenericErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::GenericError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::GenericError>(ErrorValue))
			{
				case GenericError::BadParameter:
						return "Bad parameter supplied";
					break;
				case GenericError::CouldNotCreateHandle:
						return "Operating system could not create the requested handle";
					break;
				case GenericError::EndOfFile:
						return "End of file";
					break;
				case GenericError::IndexOutOfRange:
						return "Index out of range";
					break;
				case GenericError::NoDataAvailable:
						return "No data available";
					break;
				case GenericError::OperationCanceled:
						return "The asynchronous operation was cancelled before it completed";
					break;
				case GenericError::OperationError:
						return "The asynchronous operation produced an error before it completed";
					break;
				case GenericError::QueueClosed:
						return "Operation could not be started as the service queue was missing or destroyed";
					break;
				case GenericError::SDKAlreadyInitialized:
						return "mod.io SDK was already initialized";
					break;
				case GenericError::SDKNotInitialized:
						return "mod.io SDK was not initialized";
					break;
				default:
					return "Unknown GenericError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& GenericErrorCategory()
	{
		static GenericErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(GenericError e)
	{
		return { static_cast<int>(e), GenericErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, GenericError B)
	{
		return A.category() == GenericErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, GenericError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, GenericError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class SystemError
	{
		UnknownSystemError = 22017
	};

	/// @docnone
	struct SystemErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::SystemError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::SystemError>(ErrorValue))
			{
				case SystemError::UnknownSystemError:
						return "A low-level system error occured, refer to the logs for code and location";
					break;
				default:
					return "Unknown SystemError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& SystemErrorCategory()
	{
		static SystemErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(SystemError e)
	{
		return { static_cast<int>(e), SystemErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, SystemError B)
	{
		return A.category() == SystemErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, SystemError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, SystemError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class ZlibError
	{
		EndOfStream = 22273,
		IncompleteLengthSet = 22274,
		InvalidBitLengthRepeat = 22275,
		InvalidBlockType = 22276,
		InvalidCodeLengths = 22277,
		InvalidDistance = 22278,
		InvalidDistanceCode = 22279,
		InvalidLiteralLength = 22280,
		InvalidStoredLength = 22281,
		MissingEOB = 22282,
		NeedBuffers = 22283,
		OverSubscribedLength = 22284,
		StreamError = 22285,
		TooManySymbols = 22286
	};

	/// @docnone
	struct ZlibErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::ZlibError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::ZlibError>(ErrorValue))
			{
				case ZlibError::EndOfStream:
						return "End of deflate stream";
					break;
				case ZlibError::IncompleteLengthSet:
						return "Incomplete length set";
					break;
				case ZlibError::InvalidBitLengthRepeat:
						return "Invalid bit length repeat";
					break;
				case ZlibError::InvalidBlockType:
						return "Invalid block type";
					break;
				case ZlibError::InvalidCodeLengths:
						return "Invalid code lengths";
					break;
				case ZlibError::InvalidDistance:
						return "Invalid distance";
					break;
				case ZlibError::InvalidDistanceCode:
						return "Invalid distance code";
					break;
				case ZlibError::InvalidLiteralLength:
						return "Invalid literal length";
					break;
				case ZlibError::InvalidStoredLength:
						return "Invalid store block length";
					break;
				case ZlibError::MissingEOB:
						return "Missing end-of-block marker";
					break;
				case ZlibError::NeedBuffers:
						return "Need more input data";
					break;
				case ZlibError::OverSubscribedLength:
						return "Over-subscribed length";
					break;
				case ZlibError::StreamError:
						return "Stream error";
					break;
				case ZlibError::TooManySymbols:
						return "Too many symbols";
					break;
				default:
					return "Unknown ZlibError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& ZlibErrorCategory()
	{
		static ZlibErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(ZlibError e)
	{
		return { static_cast<int>(e), ZlibErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, ZlibError B)
	{
		return A.category() == ZlibErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, ZlibError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, ZlibError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class ModManagementError
	{
		AlreadySubscribed = 22529,
		InstallOrUpdateCancelled = 22530,
		ModBeingProcessed  = 22531,
		ModManagementAlreadyEnabled = 22532,
		ModManagementDisabled = 22533,
		NoPendingWork = 22534,
		TempModSetNotInitialized = 22535,
		UploadCancelled = 22536
	};

	/// @docnone
	struct ModManagementErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::ModManagementError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::ModManagementError>(ErrorValue))
			{
				case ModManagementError::AlreadySubscribed:
						return "The user is already subscribed to the specified mod";
					break;
				case ModManagementError::InstallOrUpdateCancelled:
						return "The current mod installation or update was cancelled";
					break;
				case ModManagementError::ModBeingProcessed :
						return "The specified mod's files are currently being updated by the SDK. Please try again later.";
					break;
				case ModManagementError::ModManagementAlreadyEnabled:
						return "Mod management was already enabled and the callback remains unchanged.";
					break;
				case ModManagementError::ModManagementDisabled:
						return "Could not perform operation: Mod management is disabled and mod collection is locked";
					break;
				case ModManagementError::NoPendingWork:
						return "Internal: No mods require processing for this iteration";
					break;
				case ModManagementError::TempModSetNotInitialized:
						return "Temporary mod set was not initialized. Please call InitTempModSet.";
					break;
				case ModManagementError::UploadCancelled:
						return "The current modfile upload was cancelled";
					break;
				default:
					return "Unknown ModManagementError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& ModManagementErrorCategory()
	{
		static ModManagementErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(ModManagementError e)
	{
		return { static_cast<int>(e), ModManagementErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, ModManagementError B)
	{
		return A.category() == ModManagementErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, ModManagementError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, ModManagementError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class ModValidationError
	{
		ModDirectoryNotFound = 22785,
		NoFilesFoundForMod = 22786
	};

	/// @docnone
	struct ModValidationErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::ModValidationError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::ModValidationError>(ErrorValue))
			{
				case ModValidationError::ModDirectoryNotFound:
						return "Mod directory does not exist";
					break;
				case ModValidationError::NoFilesFoundForMod:
						return "Mod directory does not contain any files";
					break;
				default:
					return "Unknown ModValidationError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& ModValidationErrorCategory()
	{
		static ModValidationErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(ModValidationError e)
	{
		return { static_cast<int>(e), ModValidationErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, ModValidationError B)
	{
		return A.category() == ModValidationErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, ModValidationError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, ModValidationError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class MonetizationError
	{
		DisplayPriceIncorrect = 23041,
		GameMonetizationNotEnabled = 23042,
		IncorrectDisplayPrice = 23043,
		InsufficientFunds = 23044,
		ItemAlreadyOwned = 23045,
		MonetizationAuthenticationFailed = 23046,
		PaymentFailed = 23047,
		RetryEntitlements = 23048,
		UserMonetizationDisabled = 23049,
		UserMonetizationNotConfigured = 23050,
		WalletFetchFailed = 23051
	};

	/// @docnone
	struct MonetizationErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::MonetizationError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::MonetizationError>(ErrorValue))
			{
				case MonetizationError::DisplayPriceIncorrect:
						return "The display price for the mod was out-of-date or incorrect. Please retry with the correct display price.";
					break;
				case MonetizationError::GameMonetizationNotEnabled:
						return "The game does not have active monetization.";
					break;
				case MonetizationError::IncorrectDisplayPrice:
						return "The given display price does not match the price of the mod.";
					break;
				case MonetizationError::InsufficientFunds:
						return "The account has insufficent funds to make this purchase.";
					break;
				case MonetizationError::ItemAlreadyOwned:
						return "The account already owns this item.";
					break;
				case MonetizationError::MonetizationAuthenticationFailed:
						return "A failure has occured when trying to authenticate with the monetization system.";
					break;
				case MonetizationError::PaymentFailed:
						return "The payment transaction failed. Please try again later.";
					break;
				case MonetizationError::RetryEntitlements:
						return "Some entitlements could not be verified. Please try again.";
					break;
				case MonetizationError::UserMonetizationDisabled:
						return "The account does not have monetization enabled.";
					break;
				case MonetizationError::UserMonetizationNotConfigured:
						return "The account has not been created with monetization.";
					break;
				case MonetizationError::WalletFetchFailed:
						return "Unable to fetch the account's wallet. Please confirm the account has one";
					break;
				default:
					return "Unknown MonetizationError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& MonetizationErrorCategory()
	{
		static MonetizationErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(MonetizationError e)
	{
		return { static_cast<int>(e), MonetizationErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, MonetizationError B)
	{
		return A.category() == MonetizationErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, MonetizationError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, MonetizationError RawErrorValue )
	{
		return ec == RawErrorValue;
	}

	enum class ApiError
	{
		APIKeyForTestOnly = 11017,
		APIKeyHasNoGame = 11016,
		AlreadySubscribed = 15004,
		AlreadyUnsubscribed = 15005,
		AuthenticatedAccountHasBeenDeleted = 11006,
		BannedUserAccount = 11007,
		BinaryFileCorrupted = 13001,
		BinaryFileUnreadable = 13002,
		CannotMuteYourself = 17039,
		CannotVerifyExternalCredentials = 11032,
		CrossOriginForbidden = 10001,
		EmailLoginCodeExpired = 11012,
		EmailLoginCodeInvalid = 11014,
		ExpiredOrRevokedAccessToken = 11005,
		FailedToCompleteTheRequest = 10002,
		ForbiddenDMCA = 15000,
		ForbiddenHidden = 15001,
		ForbiddenMissingFile = 15020,
		ForbiddenTACNotAccepted = 15011,
		InsufficientPermission = 15019,
		InvalidAPIKey = 11002,
		InvalidApiVersion = 10003,
		InvalidJSON = 13004,
		MalformedAPIKey = 11001,
		MatureModsNotAllowed = 15054,
		MissingAPIKey = 11000,
		MissingContentTypeHeader = 13005,
		MissingReadPermission = 11004,
		MissingWritePermission = 11003,
		ModfileNoUploadPermission = 15006,
		ModioOutage = 10000,
		MonetizationAuthentication = 900002,
		MonetizationGameMonetizationNotEnabled = 900022,
		MonetizationInMaintenance = 900012,
		MonetizationIncorrectDisplayPrice = 900035,
		MonetizationInsufficientFunds = 900049,
		MonetizationItemAlreadyOwned = 900034,
		MonetizationPaymentFailed = 900030,
		MonetizationUnableToCommunicate = 900001,
		MonetizationUnexpectedError = 900000,
		MonetizationWalletFetchFailed = 900008,
		MuteUserNotFound = 17000,
		OpenIDNotConfigured = 11086,
		Ratelimited = 11008,
		RatelimitedSameEndpoint = 11009,
		ReportedEntityUnavailable = 15030,
		RequestedCommentNotFound = 15026,
		RequestedGameDeleted = 14006,
		RequestedGameNotFound = 14001,
		RequestedInvalidResponseFormat = 13007,
		RequestedModDeleted = 15023,
		RequestedModNotFound = 15022,
		RequestedModfileNotFound = 15010,
		RequestedResourceNotFound = 14000,
		RequestedUserNotFound = 21000,
		SubmitReportRightsRevoked = 15029,
		UnsupportedContentTypeHeader = 13006,
		UserExistingModRating = 15028,
		UserMonetizationDisabled = 900015,
		UserMonetizationNotConfigured = 900007,
		UserNoAcceptTermsOfUse = 11074,
		UserNoModRating = 15043,
		ValidationErrors = 13009
	};

	/// @docnone
	struct ApiErrorCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::ApiError"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::ApiError>(ErrorValue))
			{
				case ApiError::APIKeyForTestOnly:
						return "The api_key supplied in the request is for test environment purposes only and cannot be used for this functionality.";
					break;
				case ApiError::APIKeyHasNoGame:
						return "The api_key supplied in the request must be associated with a game.";
					break;
				case ApiError::AlreadySubscribed:
						return "The authenticated user is already subscribed to the mod.";
					break;
				case ApiError::AlreadyUnsubscribed:
						return "The authenticated user is not subscribed to the mod.";
					break;
				case ApiError::AuthenticatedAccountHasBeenDeleted:
						return "Authenticated user account has been deleted.";
					break;
				case ApiError::BannedUserAccount:
						return "Authenticated user account has been banned by mod.io admins.";
					break;
				case ApiError::BinaryFileCorrupted:
						return "The submitted binary file is corrupted.";
					break;
				case ApiError::BinaryFileUnreadable:
						return "The submitted binary file is unreadable.";
					break;
				case ApiError::CannotMuteYourself:
						return "You cannot mute yourself.";
					break;
				case ApiError::CannotVerifyExternalCredentials:
						return "mod.io was unable to verify the credentials against the external service provider.";
					break;
				case ApiError::CrossOriginForbidden:
						return "Cross-origin request forbidden.";
					break;
				case ApiError::EmailLoginCodeExpired:
						return "Email login code has expired. Please request a new login code.";
					break;
				case ApiError::EmailLoginCodeInvalid:
						return "Email login code is invalid.";
					break;
				case ApiError::ExpiredOrRevokedAccessToken:
						return "Access token is expired, or has been revoked.";
					break;
				case ApiError::FailedToCompleteTheRequest:
						return "mod.io failed to complete the request, please try again. (rare)";
					break;
				case ApiError::ForbiddenDMCA:
						return "This mod is currently under DMCA and the user cannot be subscribed to it.";
					break;
				case ApiError::ForbiddenHidden:
						return "This mod is hidden and the user cannot be subscribed to it.";
					break;
				case ApiError::ForbiddenMissingFile:
						return "This mod is missing a file and cannot be subscribed to.";
					break;
				case ApiError::ForbiddenTACNotAccepted:
						return "The item has not been accepted and can not be purchased at this time.";
					break;
				case ApiError::InsufficientPermission:
						return "The authenticated user does not have permission to delete this mod. This action is restricted to team managers and administrators only.";
					break;
				case ApiError::InvalidAPIKey:
						return "api_key supplied is invalid.";
					break;
				case ApiError::InvalidApiVersion:
						return "API version supplied is invalid.";
					break;
				case ApiError::InvalidJSON:
						return "You have used the input_json parameter with semantically incorrect JSON.";
					break;
				case ApiError::MalformedAPIKey:
						return "api_key supplied is malformed.";
					break;
				case ApiError::MatureModsNotAllowed:
						return "This game does not allow mature mods.";
					break;
				case ApiError::MissingAPIKey:
						return "api_key is missing from your request.";
					break;
				case ApiError::MissingContentTypeHeader:
						return "The Content-Type header is missing from your request.";
					break;
				case ApiError::MissingReadPermission:
						return "Access token is missing the read scope to perform the request.";
					break;
				case ApiError::MissingWritePermission:
						return "Access token is missing the write scope to perform the request.";
					break;
				case ApiError::ModfileNoUploadPermission:
						return "The authenticated user does not have permission to upload modfiles for the specified mod. Ensure the user is a team manager or administrator.";
					break;
				case ApiError::ModioOutage:
						return "mod.io is currently experiencing an outage. (rare)";
					break;
				case ApiError::MonetizationAuthentication:
						return "A failure has occured when trying to authenticate with the monetization system.";
					break;
				case ApiError::MonetizationGameMonetizationNotEnabled:
						return "The game does not have active monetization.";
					break;
				case ApiError::MonetizationInMaintenance:
						return "The monetization is currently in maintance mode. Please try again later.";
					break;
				case ApiError::MonetizationIncorrectDisplayPrice:
						return "The given display price does not match the price of the mod.";
					break;
				case ApiError::MonetizationInsufficientFunds:
						return "The account has insufficent funds to make this purchase.";
					break;
				case ApiError::MonetizationItemAlreadyOwned:
						return "The account already owns this item.";
					break;
				case ApiError::MonetizationPaymentFailed:
						return "The payment transaction failed. Please try again later.";
					break;
				case ApiError::MonetizationUnableToCommunicate:
						return "Unable to communicate with the monetization system. Please try again later.";
					break;
				case ApiError::MonetizationUnexpectedError:
						return "An un expected error during a purchase transaction has occured. Please try again later.";
					break;
				case ApiError::MonetizationWalletFetchFailed:
						return "Unable to fetch the accounts' wallet. Please confirm the account has one";
					break;
				case ApiError::MuteUserNotFound:
						return "The user with the supplied UserID could not be found.";
					break;
				case ApiError::OpenIDNotConfigured:
						return "You must configure your OpenID config for your game in your game authentication settings before being able to authenticate users.";
					break;
				case ApiError::Ratelimited:
						return "You have been ratelimited for making too many requests. See Rate Limiting.";
					break;
				case ApiError::RatelimitedSameEndpoint:
						return "You have been ratelimited from calling this endpoint again, for making too many requests. See Rate Limiting.";
					break;
				case ApiError::ReportedEntityUnavailable:
						return "The specified resource is not able to be reported at this time, this is potentially due to the resource in question being removed.";
					break;
				case ApiError::RequestedCommentNotFound:
						return "The requested comment could not be found.";
					break;
				case ApiError::RequestedGameDeleted:
						return "The requested game has been deleted.";
					break;
				case ApiError::RequestedGameNotFound:
						return "The requested game could not be found.";
					break;
				case ApiError::RequestedInvalidResponseFormat:
						return "You have requested a response format that is not supported (JSON only).";
					break;
				case ApiError::RequestedModDeleted:
						return "The requested mod has been deleted.";
					break;
				case ApiError::RequestedModNotFound:
						return "The requested mod could not be found.";
					break;
				case ApiError::RequestedModfileNotFound:
						return "The requested modfile could not be found.";
					break;
				case ApiError::RequestedResourceNotFound:
						return "The requested resource does not exist.";
					break;
				case ApiError::RequestedUserNotFound:
						return "The requested user could not be found.";
					break;
				case ApiError::SubmitReportRightsRevoked:
						return "The authenticated user does not have permission to submit reports on mod.io due to their access being revoked.";
					break;
				case ApiError::UnsupportedContentTypeHeader:
						return "The Content-Type header is not supported for this endpoint.";
					break;
				case ApiError::UserExistingModRating:
						return "The authenticated user has already submitted a rating for this mod.";
					break;
				case ApiError::UserMonetizationDisabled:
						return "The account has been disabled from monetization.";
					break;
				case ApiError::UserMonetizationNotConfigured:
						return "The account has not been created with monetization.";
					break;
				case ApiError::UserNoAcceptTermsOfUse:
						return "The user has not agreed to the mod.io Terms of Use. Please see terms_agreed parameter description and the Terms endpoint for more information.";
					break;
				case ApiError::UserNoModRating:
						return "The authenticated user cannot clear the mod rating as none exists.";
					break;
				case ApiError::ValidationErrors:
						return "The request contains validation errors for the data supplied. See the attached errors field within the Error Object to determine which input failed.";
					break;
				default:
					return "Unknown ApiError error";
			}
		}
	};

	/// @docnone
	inline const std::error_category& ApiErrorCategory()
	{
		static ApiErrorCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_code make_error_code(ApiError e)
	{
		return { static_cast<int>(e), ApiErrorCategory() };
	}

	/// @docnone
	inline bool operator==(std::error_code A, ApiError B)
	{
		return A.category() == ApiErrorCategory() && A.value() == static_cast<int>(B);
	}

	/// @docnone
	inline bool operator!=(std::error_code A, ApiError B)
	{
		return ! (A == B);
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, ApiError RawErrorValue )
	{
		return ec == RawErrorValue;
	}


	namespace Detail
	{
		/// @docinternal
		/// @brief Helper method to deserialize a numerical ID to an error category
		/// will only work with Modio specific error categories
		inline const std::error_category& GetModioErrorCategoryByID(std::uint64_t CategoryID)
		{
			switch (CategoryID)
			{
				case 1:
					return HttpErrorCategory();
				break;
				case 2:
					return FilesystemErrorCategory();
				break;
				case 3:
					return UserAuthErrorCategory();
				break;
				case 4:
					return UserDataErrorCategory();
				break;
				case 5:
					return ArchiveErrorCategory();
				break;
				case 6:
					return GenericErrorCategory();
				break;
				case 7:
					return SystemErrorCategory();
				break;
				case 8:
					return ZlibErrorCategory();
				break;
				case 9:
					return ModManagementErrorCategory();
				break;
				case 10:
					return ModValidationErrorCategory();
				break;
				case 11:
					return MonetizationErrorCategory();
				break;
				case 12:
					return ApiErrorCategory();
				break;
				default:
					return std::system_category();
			}

		}
		/// @docinternal
		/// @brief Helper method to serialize a modio error category to a numerical ID
		inline const std::uint64_t ModioErrorCategoryID(const std::error_category& Category)
		{
			if (Category ==  HttpErrorCategory())
			{
					return 1;
			}
			if (Category ==  FilesystemErrorCategory())
			{
					return 2;
			}
			if (Category ==  UserAuthErrorCategory())
			{
					return 3;
			}
			if (Category ==  UserDataErrorCategory())
			{
					return 4;
			}
			if (Category ==  ArchiveErrorCategory())
			{
					return 5;
			}
			if (Category ==  GenericErrorCategory())
			{
					return 6;
			}
			if (Category ==  SystemErrorCategory())
			{
					return 7;
			}
			if (Category ==  ZlibErrorCategory())
			{
					return 8;
			}
			if (Category ==  ModManagementErrorCategory())
			{
					return 9;
			}
			if (Category ==  ModValidationErrorCategory())
			{
					return 10;
			}
			if (Category ==  MonetizationErrorCategory())
			{
					return 11;
			}
			if (Category ==  ApiErrorCategory())
			{
					return 12;
			}
			return 0;
		}

	}

	/// @docpublic
	/// @brief Enum describing the different conditions a Modio::ErrorCode can satisfy
	/// Check if a Modio::ErrorCode meets a particular condition using <<ErrorCodeMatches>>
	enum class ErrorConditionTypes
	{
		/// @brief When this condition is true, the error code represents an error occurring at the mod.io server.
		ModioServiceError = 1,
		/// @brief When this condition is true, the error code represents a connection or HTTP error between the client and the mod.io server.
		NetworkError = 2,
		/// @brief When this condition is true, the error code indicates the SDK's configuration is not valid - the game ID or API key are incorrect or the game has been deleted.
		ConfigurationError = 3,
		/// @brief When this condition is true, the error code indicates the arguments passed to the function have failed validation or were otherwise invalid.
		InvalidArgsError = 4,
		/// @brief When this condition is true, the error code indicates a permission or IO error when accessing local filesystem data.
		FilesystemError = 5,
		/// @brief When this condition is true, the error code represents an internal SDK error - please inform mod.io of the error code value.
		InternalError = 6,
		/// @brief When this condition is true, the error ref returned by the API indicates an implicit success because the operation has already been performed (ie a no-op is success).
		ApiErrorRefSuccess = 7,
		/// @brief When this condition is true, the error code represents a temporary error with installation, such as a network interruption. The mod installation can be reattempted at a later point this session
		ModInstallRetryableError = 8,
		/// @brief When this condition is true, the error code represents an error during installation that may be resolved during next SDK initialization, and will be deferred until then. This category is now deprecated as deferral is the default retry behaviour for mod installation.
		ModInstallDeferredError = 9,
		/// @brief When this condition is true, the error code represents an error during uninstallation that may be resolved during the next SDK session, and will be deferred until then.
		ModDeleteDeferredError = 10,
		/// @brief When this condition is true, the error code represents an error during installation that indicates future installation will not be possible, such as a deleted mod, and so will not be retried at all.
		ModInstallUnrecoverableError = 11,
		/// @brief When this condition is true, the error code indicates that a specified game, mod, user, media file or mod file was not found.
		EntityNotFoundError = 12,
		/// @brief When this condition is true, the error code indicates that the user has not yet accepted the mod.io Terms of Use.
		UserTermsOfUseError = 13,
		/// @brief When this condition is true, the error code indicates that a report for the specified content could not be submitted.
		SubmitReportError = 14,
		/// @brief When this condition is true, the error code indicates that a user is not authenticated.
		UserNotAuthenticatedError = 15,
		/// @brief When this condition is true, the error code indicates that the SDK has not been initialized.
		SDKNotInitialized = 16,
		/// @brief When this condition is true, the error code indicates that the user is already authenticated.
		UserAlreadyAuthenticatedError = 17,
		/// @brief When this condition is true, the error code indicates that a low-level system error occurred outside of mod.io SDK control.
		SystemError = 18,
		/// @brief When this condition is true, the error code indicates that the asynchronous operation was cancelled before it completed.
		OperationCanceled = 19,
		/// @brief When this condition is true, the error code indicates that Mod Management has not been enabled.
		ModManagementDisabled = 20,
		/// @brief Too many requests made to the mod.io API within the rate-limiting window. Please wait and try again.
		RateLimited = 21,
		/// @brief The specified mod's files are currently being updated by the SDK. Please try again later.
		ModBeingProcessed = 22,
		/// @brief There is insufficient space to install the mod. Please free up space and try again.
		InsufficientSpace = 23,
		/// @brief When this condition is true, the error code indicates that the SDK has already been initialized.
		SDKAlreadyInitialized = 24,
		/// @brief When this condition is true, the error code indicates that Mod Management has already been enabled.
		ModManagementAlreadyEnabled = 25,
		/// @brief When this condition is true, the error code indicates that the current user does not have the required permissions for this operation.
		InsufficientPermissions = 26,
		/// @brief The email login code is incorrect or has expired.
		EmailLoginCodeInvalid = 27,
		/// @brief The specified mod is already subscribed to.
		AlreadySubscribed = 28,
		/// @brief The current mod installation or update was cancelled.
		InstallOrUpdateCancelled = 29,
		/// @brief The current modfile upload was cancelled.
		UploadCancelled = 30,
		/// @brief TempModSet need to be initialized first, call InitTempModSet.
		TempModSetNotInitialized = 31,
		/// @brief An error occurred while performing a monetization operation.
		MonetizationOperationError = 32,
		/// @brief The transaction requires a payment but it could not be fulfilled. Please retry with funds on the wallet
		PaymentTransactionFailed = 33,
		/// @brief The display price for the mod is out-of-date or incorrect. Please retry with the correct display price.
		IncorrectPrice = 34,
		/// @brief The authenticated user already has acquired this item
		ItemAlreadyOwned = 35
	};

	/// @docnone
	struct ErrorConditionCategoryImpl : std::error_category
	{
		inline const char* name() const noexcept override { return "Modio::ErrorConditionCategory"; }
		inline std::string message(int ErrorValue) const override
		{
			switch (static_cast<Modio::ErrorConditionTypes>(ErrorValue))
			{
				case ErrorConditionTypes::ModioServiceError:
					return "When this condition is true, the error code represents an error occurring at the mod.io server.";
				break;
				case ErrorConditionTypes::NetworkError:
					return "When this condition is true, the error code represents a connection or HTTP error between the client and the mod.io server.";
				break;
				case ErrorConditionTypes::ConfigurationError:
					return "When this condition is true, the error code indicates the SDK's configuration is not valid - the game ID or API key are incorrect or the game has been deleted.";
				break;
				case ErrorConditionTypes::InvalidArgsError:
					return "When this condition is true, the error code indicates the arguments passed to the function have failed validation or were otherwise invalid.";
				break;
				case ErrorConditionTypes::FilesystemError:
					return "When this condition is true, the error code indicates a permission or IO error when accessing local filesystem data.";
				break;
				case ErrorConditionTypes::InternalError:
					return "When this condition is true, the error code represents an internal SDK error - please inform mod.io of the error code value.";
				break;
				case ErrorConditionTypes::ApiErrorRefSuccess:
					return "When this condition is true, the error ref returned by the API indicates an implicit success because the operation has already been performed (ie a no-op is success).";
				break;
				case ErrorConditionTypes::ModInstallRetryableError:
					return "When this condition is true, the error code represents a temporary error with installation, such as a network interruption. The mod installation can be reattempted at a later point this session";
				break;
				case ErrorConditionTypes::ModInstallDeferredError:
					return "When this condition is true, the error code represents an error during installation that may be resolved during next SDK initialization, and will be deferred until then. This category is now deprecated as deferral is the default retry behaviour for mod installation.";
				break;
				case ErrorConditionTypes::ModDeleteDeferredError:
					return "When this condition is true, the error code represents an error during uninstallation that may be resolved during the next SDK session, and will be deferred until then.";
				break;
				case ErrorConditionTypes::ModInstallUnrecoverableError:
					return "When this condition is true, the error code represents an error during installation that indicates future installation will not be possible, such as a deleted mod, and so will not be retried at all.";
				break;
				case ErrorConditionTypes::EntityNotFoundError:
					return "When this condition is true, the error code indicates that a specified game, mod, user, media file or mod file was not found.";
				break;
				case ErrorConditionTypes::UserTermsOfUseError:
					return "When this condition is true, the error code indicates that the user has not yet accepted the mod.io Terms of Use.";
				break;
				case ErrorConditionTypes::SubmitReportError:
					return "When this condition is true, the error code indicates that a report for the specified content could not be submitted.";
				break;
				case ErrorConditionTypes::UserNotAuthenticatedError:
					return "When this condition is true, the error code indicates that a user is not authenticated.";
				break;
				case ErrorConditionTypes::SDKNotInitialized:
					return "When this condition is true, the error code indicates that the SDK has not been initialized.";
				break;
				case ErrorConditionTypes::UserAlreadyAuthenticatedError:
					return "When this condition is true, the error code indicates that the user is already authenticated.";
				break;
				case ErrorConditionTypes::SystemError:
					return "When this condition is true, the error code indicates that a low-level system error occurred outside of mod.io SDK control.";
				break;
				case ErrorConditionTypes::OperationCanceled:
					return "When this condition is true, the error code indicates that the asynchronous operation was cancelled before it completed.";
				break;
				case ErrorConditionTypes::ModManagementDisabled:
					return "When this condition is true, the error code indicates that Mod Management has not been enabled.";
				break;
				case ErrorConditionTypes::RateLimited:
					return "Too many requests made to the mod.io API within the rate-limiting window. Please wait and try again.";
				break;
				case ErrorConditionTypes::ModBeingProcessed:
					return "The specified mod's files are currently being updated by the SDK. Please try again later.";
				break;
				case ErrorConditionTypes::InsufficientSpace:
					return "There is insufficient space to install the mod. Please free up space and try again.";
				break;
				case ErrorConditionTypes::SDKAlreadyInitialized:
					return "When this condition is true, the error code indicates that the SDK has already been initialized.";
				break;
				case ErrorConditionTypes::ModManagementAlreadyEnabled:
					return "When this condition is true, the error code indicates that Mod Management has already been enabled.";
				break;
				case ErrorConditionTypes::InsufficientPermissions:
					return "When this condition is true, the error code indicates that the current user does not have the required permissions for this operation.";
				break;
				case ErrorConditionTypes::EmailLoginCodeInvalid:
					return "The email login code is incorrect or has expired.";
				break;
				case ErrorConditionTypes::AlreadySubscribed:
					return "The specified mod is already subscribed to.";
				break;
				case ErrorConditionTypes::InstallOrUpdateCancelled:
					return "The current mod installation or update was cancelled.";
				break;
				case ErrorConditionTypes::UploadCancelled:
					return "The current modfile upload was cancelled.";
				break;
				case ErrorConditionTypes::TempModSetNotInitialized:
					return "TempModSet need to be initialized first, call InitTempModSet.";
				break;
				case ErrorConditionTypes::MonetizationOperationError:
					return "An error occurred while performing a monetization operation.";
				break;
				case ErrorConditionTypes::PaymentTransactionFailed:
					return "The transaction requires a payment but it could not be fulfilled. Please retry with funds on the wallet";
				break;
				case ErrorConditionTypes::IncorrectPrice:
					return "The display price for the mod is out-of-date or incorrect. Please retry with the correct display price.";
				break;
				case ErrorConditionTypes::ItemAlreadyOwned:
					return "The authenticated user already has acquired this item";
				break;
				default:
					return "Unknown error condition";
			}
		}

		inline bool equivalent(const std::error_code& ec, int cond) const noexcept override 
		{
			switch (static_cast<Modio::ErrorConditionTypes>(cond))
			{
				case ErrorConditionTypes::ModioServiceError:
					if (ec == Modio::ApiError::ModioOutage)
					{
						return true;
					}

					if (ec == Modio::HttpError::ServersOverloaded)
					{
						return true;
					}

					if (ec == Modio::HttpError::RateLimited)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationUnexpectedError)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationUnableToCommunicate)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationAuthentication)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationInMaintenance)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::NetworkError:


					if (ec == Modio::HttpError::CannotOpenConnection)
					{
						return true;
					}

					if (ec == Modio::HttpError::DownloadNotPermitted)
					{
						return true;
					}

					if (ec == Modio::HttpError::ExcessiveRedirects)
					{
						return true;
					}

					if (ec == Modio::HttpError::HttpAlreadyInitialized)
					{
						return true;
					}

					if (ec == Modio::HttpError::HttpNotInitialized)
					{
						return true;
					}

					if (ec == Modio::HttpError::InsufficientPermissions)
					{
						return true;
					}

					if (ec == Modio::HttpError::InvalidResponse)
					{
						return true;
					}

					if (ec == Modio::HttpError::RateLimited)
					{
						return true;
					}

					if (ec == Modio::HttpError::RequestError)
					{
						return true;
					}

					if (ec == Modio::HttpError::ResourceNotAvailable)
					{
						return true;
					}

					if (ec == Modio::HttpError::SecurityConfigurationInvalid)
					{
						return true;
					}

					if (ec == Modio::HttpError::ServerClosedConnection)
					{
						return true;
					}

					if (ec == Modio::HttpError::ServerUnavailable)
					{
						return true;
					}

					if (ec == Modio::HttpError::ServersOverloaded)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::ConfigurationError:
					if (ec == Modio::ApiError::MissingAPIKey)
					{
						return true;
					}

					if (ec == Modio::ApiError::MalformedAPIKey)
					{
						return true;
					}

					if (ec == Modio::ApiError::InvalidAPIKey)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedGameNotFound)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedGameDeleted)
					{
						return true;
					}

					if (ec == Modio::ApiError::APIKeyHasNoGame)
					{
						return true;
					}

					if (ec == Modio::ApiError::APIKeyForTestOnly)
					{
						return true;
					}

					if (ec == Modio::ApiError::OpenIDNotConfigured)
					{
						return true;
					}

					if (ec == Modio::GenericError::BadParameter)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::InvalidArgsError:
					if (ec == Modio::ApiError::MalformedAPIKey)
					{
						return true;
					}

					if (ec == Modio::ApiError::InvalidAPIKey)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedGameNotFound)
					{
						return true;
					}

					if (ec == Modio::ApiError::ValidationErrors)
					{
						return true;
					}

					if (ec == Modio::ApiError::APIKeyHasNoGame)
					{
						return true;
					}

					if (ec == Modio::ApiError::APIKeyForTestOnly)
					{
						return true;
					}

					if (ec == Modio::ApiError::CannotVerifyExternalCredentials)
					{
						return true;
					}

					if (ec == Modio::GenericError::IndexOutOfRange)
					{
						return true;
					}

					if (ec == Modio::GenericError::BadParameter)
					{
						return true;
					}

					if (ec == Modio::ApiError::CannotMuteYourself)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationPaymentFailed)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationIncorrectDisplayPrice)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::FilesystemError:


					if (ec == Modio::FilesystemError::DirectoryNotEmpty)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::DirectoryNotFound)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::FileLocked)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::FileNotFound)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::InsufficientSpace)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::NoPermission)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::ReadError)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::UnableToCreateFile)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::UnableToCreateFolder)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::WriteError)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::InternalError:
					if (ec == Modio::ApiError::InvalidApiVersion)
					{
						return true;
					}

					if (ec == Modio::ApiError::MissingContentTypeHeader)
					{
						return true;
					}

					if (ec == Modio::ApiError::UnsupportedContentTypeHeader)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedInvalidResponseFormat)
					{
						return true;
					}

					if (ec == Modio::ApiError::InvalidJSON)
					{
						return true;
					}

					if (ec == Modio::ApiError::CrossOriginForbidden)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::ApiErrorRefSuccess:
					if (ec == Modio::ApiError::AlreadySubscribed)
					{
						return true;
					}

					if (ec == Modio::ApiError::AlreadyUnsubscribed)
					{
						return true;
					}

					if (ec == Modio::ApiError::UserExistingModRating)
					{
						return true;
					}

					if (ec == Modio::ApiError::UserNoModRating)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationItemAlreadyOwned)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::ModInstallRetryableError:
					if (ec == Modio::ApiError::ModioOutage)
					{
						return true;
					}

					if (ec == Modio::ApiError::FailedToCompleteTheRequest)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationUnexpectedError)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationUnableToCommunicate)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationInMaintenance)
					{
						return true;
					}



					if (ec == Modio::HttpError::CannotOpenConnection)
					{
						return true;
					}

					if (ec == Modio::HttpError::DownloadNotPermitted)
					{
						return true;
					}

					if (ec == Modio::HttpError::ExcessiveRedirects)
					{
						return true;
					}

					if (ec == Modio::HttpError::HttpAlreadyInitialized)
					{
						return true;
					}

					if (ec == Modio::HttpError::HttpNotInitialized)
					{
						return true;
					}

					if (ec == Modio::HttpError::InsufficientPermissions)
					{
						return true;
					}

					if (ec == Modio::HttpError::InvalidResponse)
					{
						return true;
					}

					if (ec == Modio::HttpError::RateLimited)
					{
						return true;
					}

					if (ec == Modio::HttpError::RequestError)
					{
						return true;
					}

					if (ec == Modio::HttpError::ResourceNotAvailable)
					{
						return true;
					}

					if (ec == Modio::HttpError::SecurityConfigurationInvalid)
					{
						return true;
					}

					if (ec == Modio::HttpError::ServerClosedConnection)
					{
						return true;
					}

					if (ec == Modio::HttpError::ServerUnavailable)
					{
						return true;
					}

					if (ec == Modio::HttpError::ServersOverloaded)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::ModInstallDeferredError:
					if (ec == Modio::GenericError::OperationCanceled)
					{
						return true;
					}

					if (ec == Modio::GenericError::CouldNotCreateHandle)
					{
						return true;
					}

					if (ec == Modio::GenericError::QueueClosed)
					{
						return true;
					}



					if (ec == Modio::FilesystemError::DirectoryNotEmpty)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::DirectoryNotFound)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::FileLocked)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::FileNotFound)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::InsufficientSpace)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::NoPermission)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::ReadError)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::UnableToCreateFile)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::UnableToCreateFolder)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::WriteError)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::ModDeleteDeferredError:
					if (ec == std::errc::directory_not_empty)
					{
						return true;
					}

					if (ec == std::errc::no_such_file_or_directory)
					{
						return true;
					}

					if (ec == Modio::GenericError::OperationCanceled)
					{
						return true;
					}

					if (ec == Modio::GenericError::CouldNotCreateHandle)
					{
						return true;
					}

					if (ec == Modio::GenericError::QueueClosed)
					{
						return true;
					}



					if (ec == Modio::FilesystemError::DirectoryNotEmpty)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::DirectoryNotFound)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::FileLocked)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::FileNotFound)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::InsufficientSpace)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::NoPermission)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::ReadError)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::UnableToCreateFile)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::UnableToCreateFolder)
					{
						return true;
					}

					if (ec == Modio::FilesystemError::WriteError)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::ModInstallUnrecoverableError:
					if (ec == Modio::ApiError::RequestedGameNotFound)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedGameDeleted)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedModNotFound)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedModDeleted)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedModfileNotFound)
					{
						return true;
					}



					if (ec == Modio::ZlibError::EndOfStream)
					{
						return true;
					}

					if (ec == Modio::ZlibError::IncompleteLengthSet)
					{
						return true;
					}

					if (ec == Modio::ZlibError::InvalidBitLengthRepeat)
					{
						return true;
					}

					if (ec == Modio::ZlibError::InvalidBlockType)
					{
						return true;
					}

					if (ec == Modio::ZlibError::InvalidCodeLengths)
					{
						return true;
					}

					if (ec == Modio::ZlibError::InvalidDistance)
					{
						return true;
					}

					if (ec == Modio::ZlibError::InvalidDistanceCode)
					{
						return true;
					}

					if (ec == Modio::ZlibError::InvalidLiteralLength)
					{
						return true;
					}

					if (ec == Modio::ZlibError::InvalidStoredLength)
					{
						return true;
					}

					if (ec == Modio::ZlibError::MissingEOB)
					{
						return true;
					}

					if (ec == Modio::ZlibError::NeedBuffers)
					{
						return true;
					}

					if (ec == Modio::ZlibError::OverSubscribedLength)
					{
						return true;
					}

					if (ec == Modio::ZlibError::StreamError)
					{
						return true;
					}

					if (ec == Modio::ZlibError::TooManySymbols)
					{
						return true;
					}

	

					if (ec == Modio::ArchiveError::InvalidHeader)
					{
						return true;
					}

					if (ec == Modio::ArchiveError::UnsupportedCompression)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::EntityNotFoundError:
					if (ec == Modio::ApiError::RequestedGameNotFound)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedGameDeleted)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedModNotFound)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedModDeleted)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedModfileNotFound)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedResourceNotFound)
					{
						return true;
					}

					if (ec == Modio::ApiError::MuteUserNotFound)
					{
						return true;
					}

					if (ec == Modio::ApiError::AuthenticatedAccountHasBeenDeleted)
					{
						return true;
					}

					if (ec == Modio::ApiError::UserMonetizationNotConfigured)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::UserTermsOfUseError:
					if (ec == Modio::ApiError::UserNoAcceptTermsOfUse)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::SubmitReportError:
					if (ec == Modio::ApiError::SubmitReportRightsRevoked)
					{
						return true;
					}

					if (ec == Modio::ApiError::ReportedEntityUnavailable)
					{
						return true;
					}

					if (ec == Modio::ApiError::RequestedResourceNotFound)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::UserNotAuthenticatedError:
					if (ec == Modio::UserAuthError::NoAuthToken)
					{
						return true;
					}

					if (ec == Modio::UserAuthError::StatusAuthTokenInvalid)
					{
						return true;
					}

					if (ec == Modio::UserAuthError::StatusAuthTokenMissing)
					{
						return true;
					}

					if (ec == Modio::UserAuthError::UnableToInitStorage)
					{
						return true;
					}

					if (ec == Modio::UserDataError::InvalidUser)
					{
						return true;
					}

					if (ec == Modio::ApiError::ExpiredOrRevokedAccessToken)
					{
						return true;
					}

					if (ec == Modio::ApiError::AuthenticatedAccountHasBeenDeleted)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationAuthentication)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::SDKNotInitialized:
					if (ec == Modio::GenericError::SDKNotInitialized)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::UserAlreadyAuthenticatedError:
					if (ec == Modio::UserAuthError::AlreadyAuthenticated)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::SystemError:
					if (ec == Modio::SystemError::UnknownSystemError)
					{
						return true;
					}



					if (ec == Modio::SystemError::UnknownSystemError)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::OperationCanceled:
					if (ec == Modio::GenericError::OperationCanceled)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::ModManagementDisabled:
					if (ec == Modio::ModManagementError::ModManagementDisabled)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::RateLimited:
					if (ec == Modio::HttpError::RateLimited)
					{
						return true;
					}

					if (ec == Modio::ApiError::Ratelimited)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::ModBeingProcessed:
					if (ec == Modio::ModManagementError::ModBeingProcessed)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::InsufficientSpace:
					if (ec == Modio::FilesystemError::InsufficientSpace)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::SDKAlreadyInitialized:
					if (ec == Modio::GenericError::SDKAlreadyInitialized)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::ModManagementAlreadyEnabled:
					if (ec == Modio::ModManagementError::ModManagementAlreadyEnabled)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::InsufficientPermissions:
					if (ec == Modio::HttpError::InsufficientPermissions)
					{
						return true;
					}

					if (ec == Modio::ApiError::InsufficientPermission)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::EmailLoginCodeInvalid:
					if (ec == Modio::UserAuthError::EmailLoginCodeExpired)
					{
						return true;
					}

					if (ec == Modio::UserAuthError::EmailLoginCodeInvalid)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::AlreadySubscribed:
					if (ec == Modio::ModManagementError::AlreadySubscribed)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::InstallOrUpdateCancelled:
					if (ec == Modio::ModManagementError::InstallOrUpdateCancelled)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::UploadCancelled:
					if (ec == Modio::ModManagementError::UploadCancelled)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::TempModSetNotInitialized:
					if (ec == Modio::ModManagementError::TempModSetNotInitialized)
					{
						return true;
					}


				break;
				case ErrorConditionTypes::MonetizationOperationError:
					if (ec == Modio::MonetizationError::UserMonetizationNotConfigured)
					{
						return true;
					}

					if (ec == Modio::ApiError::UserMonetizationNotConfigured)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::UserMonetizationDisabled)
					{
						return true;
					}

					if (ec == Modio::ApiError::UserMonetizationDisabled)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::PaymentFailed)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationPaymentFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::IncorrectDisplayPrice)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationIncorrectDisplayPrice)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::ItemAlreadyOwned)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationItemAlreadyOwned)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::InsufficientFunds)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationInsufficientFunds)
					{
						return true;
					}



					if (ec == Modio::MonetizationError::DisplayPriceIncorrect)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::GameMonetizationNotEnabled)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::IncorrectDisplayPrice)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::InsufficientFunds)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::ItemAlreadyOwned)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::MonetizationAuthenticationFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::PaymentFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::RetryEntitlements)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::UserMonetizationDisabled)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::UserMonetizationNotConfigured)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::WalletFetchFailed)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::PaymentTransactionFailed:
					if (ec == Modio::MonetizationError::PaymentFailed)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationPaymentFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::InsufficientFunds)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationInsufficientFunds)
					{
						return true;
					}



					if (ec == Modio::MonetizationError::DisplayPriceIncorrect)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::GameMonetizationNotEnabled)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::IncorrectDisplayPrice)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::InsufficientFunds)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::ItemAlreadyOwned)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::MonetizationAuthenticationFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::PaymentFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::RetryEntitlements)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::UserMonetizationDisabled)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::UserMonetizationNotConfigured)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::WalletFetchFailed)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::IncorrectPrice:
					if (ec == Modio::MonetizationError::IncorrectDisplayPrice)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationIncorrectDisplayPrice)
					{
						return true;
					}



					if (ec == Modio::MonetizationError::DisplayPriceIncorrect)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::GameMonetizationNotEnabled)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::IncorrectDisplayPrice)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::InsufficientFunds)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::ItemAlreadyOwned)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::MonetizationAuthenticationFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::PaymentFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::RetryEntitlements)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::UserMonetizationDisabled)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::UserMonetizationNotConfigured)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::WalletFetchFailed)
					{
						return true;
					}

	
				break;
				case ErrorConditionTypes::ItemAlreadyOwned:
					if (ec == Modio::MonetizationError::ItemAlreadyOwned)
					{
						return true;
					}

					if (ec == Modio::ApiError::MonetizationItemAlreadyOwned)
					{
						return true;
					}



					if (ec == Modio::MonetizationError::DisplayPriceIncorrect)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::GameMonetizationNotEnabled)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::IncorrectDisplayPrice)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::InsufficientFunds)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::ItemAlreadyOwned)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::MonetizationAuthenticationFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::PaymentFailed)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::RetryEntitlements)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::UserMonetizationDisabled)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::UserMonetizationNotConfigured)
					{
						return true;
					}

					if (ec == Modio::MonetizationError::WalletFetchFailed)
					{
						return true;
					}

	
				break;
			}
			return false;
		}
	};
	
	/// @docnone
	inline const std::error_category& ErrorConditionCategory()
	{
		static ErrorConditionCategoryImpl CategoryInstance;
		return CategoryInstance;
	}

	/// @docnone
	inline std::error_condition make_error_condition(Modio::ErrorConditionTypes e)
	{
		return { static_cast<int>(e), Modio::ErrorConditionCategory()};
	}

	/// @docnone
	inline bool ErrorCodeMatches(const Modio::ErrorCode& ec, Modio::ErrorConditionTypes Type)
	{
		return ec == make_error_condition(Type);
	}
	
}

// Type trait so stdlib knows ErrorConditionTypes is a list of error conditions
namespace std
{
  template <>
    struct is_error_condition_enum<Modio::ErrorConditionTypes>
    : true_type {};
}
