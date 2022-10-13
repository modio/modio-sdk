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
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/JsonWrapper.h"
#include "modio/detail/entities/ModioAvatar.h"
#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief Object representing a mod.io user profile
	struct User
	{
		/// @brief Unique id for the user
		Modio::UserID UserId = Modio::UserID(0);

		/// @brief Username of the user
		std::string Username = "";

		/// @brief Unix timestamp the user was last online
		std::int64_t DateOnline = 0;

		/// @brief URL of the user's mod.io profile
		std::string ProfileUrl = "";

		/// @brief Cached information about the user's avatar
		Modio::Detail::Avatar Avatar;

		/// @docnone
		friend bool operator==(const Modio::User& A, const Modio::User& B)
		{
			return (A.UserId == B.UserId && A.Username == B.Username && A.DateOnline == B.DateOnline &&
					A.ProfileUrl == B.ProfileUrl && A.Avatar == B.Avatar);
		}
	};

	/// @docnone
	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::User& User);
	
	/// @docnone
	MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::User& User);

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioUser.ipp"
#endif