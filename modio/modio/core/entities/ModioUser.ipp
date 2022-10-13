/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *  
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioUser.h"
#endif

#include "modio/detail/ModioJsonHelpers.h"


namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::User& User)
	{
		Detail::ParseSafe(Json, User.UserId, "id");
		Detail::ParseSafe(Json, User.Username, "username");
		Detail::ParseSafe(Json, User.DateOnline, "date_online");
		Detail::ParseSafe(Json, User.ProfileUrl, "profile_url");
		nlohmann::json AvatarJson;
		if (Detail::GetSubobjectSafe(Json, "avatar", AvatarJson))
		{
			Modio::Detail::from_json(AvatarJson, User.Avatar);
		}
	}
	
	void to_json(nlohmann::json& Json, const Modio::User& User)
	{
		Json = nlohmann::json {{"id", User.UserId},
							   {"username", User.Username},
							   {"date_online", User.DateOnline},
							   {"profile_url", User.ProfileUrl},
							   {"avatar", User.Avatar}};
	}
}