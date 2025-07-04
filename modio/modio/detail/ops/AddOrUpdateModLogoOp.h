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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/ops/mod/GetModInfoOp.h"
#include "modio/http/ModioHttpParams.h"

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class AddOrUpdateModLogoOp
		{
			asio::coroutine CoroState {};
			Modio::GameID GameID {};
			Modio::ModID ModID {};
			Modio::filesystem::path LogoPath {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};

		public:
			AddOrUpdateModLogoOp(Modio::GameID GameID, Modio::ModID ModID, Modio::filesystem::path LogoPath)
				: GameID(GameID),
				  ModID(ModID),
				  LogoPath(LogoPath)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::AddModMediaRequest.SetGameID(GameID).SetModID(ModID).AppendPayloadFile("logo",
																											  LogoPath),
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
					}
					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// After a logo update, we need to invalidate the cache in case something exists
					Modio::Detail::SDKSessionData::InvalidateModCache(ModID);

					// Also, remove the logo file stored for that mod
					RemoveFileIfExists(Modio::LogoSize::Original);
					RemoveFileIfExists(Modio::LogoSize::Thumb640);
					RemoveFileIfExists(Modio::LogoSize::Thumb1280);
					RemoveFileIfExists(Modio::LogoSize::Thumb320);

					Self.complete({});
				}
			}

			void RemoveFileIfExists(Modio::LogoSize MODIO_UNUSED_ARGUMENT(Logo))
			{
				Modio::Detail::FileService& FileService =
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();
				Modio::Optional<Modio::filesystem::path> Path = FileService.GetLogo(ModID, Modio::LogoSize::Original);
				if (Path.has_value() == true)
				{
					FileService.DeleteFile(Path.value());
				}
			}
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
