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
	#include "modio/detail/ops/mod/SubmitModChangesOp.h"
#endif

namespace Modio
{
	namespace Detail
	{
		SubmitModChangesOp::SubmitModChangesOp(Modio::ModID Mod, Modio::EditModParams Params)
		{
			EditRequestParams = Modio::Detail::EditModRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
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
				EditRequestParams = EditRequestParams.AppendPayloadValue(
					"visible", fmt::format("{}", static_cast<std::uint8_t>(Params.Visibility.value())));
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

			if (Params.CommunityOptions)
			{
				EditRequestParams = EditRequestParams.AppendPayloadValue(
					"community_options", fmt::format("{}", Params.CommunityOptions->RawValue()));
			}

			if (Params.MetadataKvp)
			{
				std::size_t Index = 0;
				for (const Modio::Metadata& metadata : *Params.MetadataKvp)
				{
					EditRequestParams = EditRequestParams.AppendPayloadValue(
						fmt::format("metadata_kvp[{}]", Index), fmt::format("{}:{}", metadata.Key, metadata.Value));
					Index++;
				}
			}
		}

	} // namespace Detail
} // namespace Modio
