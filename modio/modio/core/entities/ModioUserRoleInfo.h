/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "modio/detail/OptionalWrapper.h"
#include "modio/core/ModioCoreTypes.h"

namespace Modio
{
	/// @docpublic
	/// @brief The type of resource the role is associated with
	enum class UserRoleResourceType
	{
		/// @brief a game associated role
		Game = 0
	};

	/// @docpublic
	/// @brief	Specifies the role of the user in relation to the specified Game ID.
	struct UserRoleInfo
	{
		/// @docpublic
		/// @brief	An enum that specifies a particular users role in relation to a specific resource type (e.g. game)
		enum class UserRoleType
		{
			/// @brief the user is a member of this resource
			Member = 0
		};

		/// @docpublic
		/// @brief Constructor for a user role info object
		/// @param GameId the ID of the game related to the user role
		/// @param Role the game role specified for the current user
		UserRoleInfo(Modio::GameID GameId, Modio::UserRoleInfo::UserRoleType Role)
			: Type(Modio::UserRoleResourceType::Game),
			  Role(Role),
			  UnderlyingId(GameId)
		{
		}

		UserRoleInfo(const UserRoleInfo&) = default;
		UserRoleInfo(UserRoleInfo&&) = default;
		UserRoleInfo& operator = (const UserRoleInfo&) = default;
		UserRoleInfo& operator = (UserRoleInfo&) = default;

		/// @brief the area of involement
		Modio::UserRoleResourceType Type;

		/// @brief the role of the user within that specific game.
		Modio::UserRoleInfo::UserRoleType Role;

		/// @brief If the Modio::ResourceType is currently set to Modio::ResourceType::Game, this method will return the associated Game ID
		/// @return the game ID assoicated with the user role
		Modio::Optional<Modio::GameID> GetGameID() const
		{
			if (Type == Modio::UserRoleResourceType::Game)
			{
				return Modio::GameID(UnderlyingId);
			}
			return tl::nullopt;
		}

	private:
		std::int64_t UnderlyingId = -1LL;
	};
} // namespace Modio
