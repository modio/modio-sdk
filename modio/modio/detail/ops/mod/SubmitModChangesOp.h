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
#include "modio/core/ModioCreateModParams.h"
#include "modio/core/entities/ModioModInfo.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/impl/SDKPreconditionChecks.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SubmitModChangesOp
		{
		public:
			SubmitModChangesOp(Modio::ModID Mod, Modio::EditModParams Params)
			{
				EditRequestParams =
					Modio::Detail::EditModRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
						.SetModID(Mod)
						.AppendPayloadValue("name", Params.Name)
						.AppendPayloadValue("summary", Params.Summary)
						.AppendPayloadValue("name_id", Params.NamePath)
						.AppendPayloadValue("description", Params.Description)
						.AppendPayloadValue("homepage_url", Params.HomepageURL)
						.AppendPayloadValue("metadata_blob", Params.MetadataBlob);

				if (Params.LogoPath.has_value())
				{
					EditRequestParams = EditRequestParams.AppendPayloadFile("logo", Params.LogoPath.value());
				}
				
				if (Params.Visibility.has_value())
				{
					EditRequestParams =
						EditRequestParams.AppendPayloadValue("visible", fmt::format("{}", static_cast<std::uint8_t>(Params.Visibility.value())));
				}
				// When bVisible is removed from deprecation, remove the else if below
				else if (Params.bVisible.has_value())
				{
					EditRequestParams =
						EditRequestParams.AppendPayloadValue("visible", Params.bVisible.value() ? "1" : "0");
				}
				
				if (Params.MaturityRating.has_value())
				{
					EditRequestParams = EditRequestParams.AppendPayloadValue(
						"maturity_option", fmt::format("{}", static_cast<std::uint8_t>(Params.MaturityRating.value())));
				}

				if (Params.Tags.has_value())
				{
					int i = 0;

					if (Params.Tags->size() != 0)
					{
						for (const std::string& tag : *Params.Tags)
						{
							EditRequestParams = EditRequestParams.AppendPayloadValue(fmt::format("tags[{}]", i), tag);
							i++;
						}
					}
					else
					{
						EditRequestParams = EditRequestParams.AppendEmptyPayload("tags[]");
					}
				}
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(ResponseBuffer, EditRequestParams,
																		   CachedResponse::Disallow, std::move(Self));

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
					}

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}
					else
					{
						auto ModInfoData = TryMarshalResponse<Modio::ModInfo>(ResponseBuffer);
						if (ModInfoData.has_value())
						{
							Modio::Detail::SDKSessionData::InvalidateModCache(ModInfoData.value().ModId);
							Self.complete(ec, ModInfoData);
							return;
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
							return;
						}
					}
				}
			}

		private:
			asio::coroutine CoroutineState;
			Modio::Detail::HttpRequestParams EditRequestParams;
			Modio::Detail::DynamicBuffer ResponseBuffer;
		};
#include <asio/unyield.hpp>

		inline auto SubmitModChangesAsync(
			Modio::ModID Mod, Modio::EditModParams Params,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)> Callback)
		{
			bool FileExists = false;

			if (Params.LogoPath.has_value() == true)
			{
				// Because LogoPath was set, we need to check file existance
				FileExists = Modio::Detail::RequireFileExists(Params.LogoPath.value(), Callback);
			}
			else
			{
				// Because LogoPath is empty, the precondition is not necessary and we need
				// to continue as normal.
				FileExists = true;
			}

			if (FileExists == true)
			{
				return asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>,
										   void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>(
					Modio::Detail::SubmitModChangesOp(Mod, Params), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		}
	} // namespace Detail
} // namespace Modio
