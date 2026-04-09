/*
 *  Copyright (C) 2021-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */
#pragma once

#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/OptionalWrapper.h"
#include "modio/core/ModioSplitCompilation.h"
#include <cstdint>
namespace Modio
{
	struct InitializeOptions;
	class MODIO_NODISCARD ErrorCode;
	enum class LogLevel;
	struct ModID;
	struct GameID;
	struct ModCollectionID;
	struct UserID;
	class UserSubscriptionList;
	struct ModManagementEvent;
	struct StorageInfo;
	class ModProgressInfo;
	class ModCollectionEntry;
	struct User;
	struct AuthenticationParams;
	enum class AuthenticationProvider;
	struct Terms;
	struct ModCreationHandle;
	class CreateModParams;
	class EditModParams;
	struct ModInfo;
	class CreateModFileParams;
	class CreateSourceFileParams;
	class FilterParams;
	class ModInfoList;
	using Timestamp = std::uint64_t;
	using GalleryIndex = std::size_t;
	class ModTagOptions;
	class ModDependencyList;
	struct EmailAddress;
	struct FieldError;
	class ReportParams;
	struct EmailAuthCode;
	struct GameInfo;
	class GameInfoList;
	class UserList;
	enum class Language;

	struct UserRatingList;
	struct TransactionRecord;
	struct EntitlementParams;
	class EntitlementConsumptionStatusList;
	class EntitlementList;
	struct MetricsSessionParams;
	struct ServerInitializeOptions;
	class ModCollectionInfoList;
	struct ModCollectionInfo;
	enum class UserSubscriptionListChangeType;
	enum class AvatarSize : std::uint8_t;
	enum class LogoSize : std::uint8_t;
	enum class GallerySize : std::uint8_t;
	enum class Rating : int8_t;

	/// @docpublic
	/// @brief nullable wrapper around object of type T. Used by async functions that return values - empty on function
	/// error/failure
	/// @tparam T underlying type
	template<typename T>
	using Optional = tl::optional<T>;

} // namespace Modio
