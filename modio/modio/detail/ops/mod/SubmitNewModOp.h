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
#include "modio/detail/CoreOps.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SubmitNewModOp
		{
		public:
			SubmitNewModOp(Modio::ModCreationHandle Handle, Modio::CreateModParams Params) : Handle(Handle)
			{
				SubmitParams = Modio::Detail::AddModRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
								   .AppendPayloadFile("logo", Params.PathToLogoFile)
								   .AppendPayloadValue("name", Params.Name)
								   .AppendPayloadValue("summary", Params.Summary)
								   .AppendPayloadValue("description", Params.Description)
								   .AppendPayloadValue("name_id", Params.NamePath)
								   .AppendPayloadValue("homepage_url", Params.HomepageURL)
								   .AppendPayloadValue("metadata_blob", Params.MetadataBlob);

				if (Params.bVisible)
				{
					SubmitParams.AppendPayloadValue("visible", Params.bVisible.value() ? "1" : "0");
				}

				if (Params.Stock)
				{
					SubmitParams.AppendPayloadValue("stock", fmt::format("{}", Params.Stock.value()));
				}

				if (Params.Tags)
				{
					std::size_t Index = 0;
					for (const auto& Tag : Params.Tags.value())
					{
						SubmitParams.AppendPayloadValue(fmt::format("tags[{}]", Index), Tag);
						Index++;
					}
				}

				if (Params.MaturityRating)
				{
					SubmitParams.AppendPayloadValue(
						"maturity_option", fmt::format("{}", static_cast<std::uint8_t>(Params.MaturityRating.value())));
				}
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						ResponseBuffer, SubmitParams, CachedResponse::Disallow, std::move(Self));
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
							Modio::Detail::SDKSessionData::LinkModCreationHandle(Handle, ModInfoData->ModId);
							Self.complete(ec, ModInfoData->ModId);
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
			Modio::Detail::HttpRequestParams SubmitParams;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			Modio::ModCreationHandle Handle;
		};
#include <asio/unyield.hpp>

		inline void SubmitNewModAsync(Modio::ModCreationHandle Handle, Modio::CreateModParams Params,
									  std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModID>)> Callback)
		{
			return asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModID>)>,
									   void(Modio::ErrorCode, Modio::Optional<Modio::ModID>)>(
				Modio::Detail::SubmitNewModOp(Handle, Params), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio