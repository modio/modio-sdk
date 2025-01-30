/*
 *  Copyright (C) 2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioModDependencySerialization.h"
#include "modio/detail/ModioJsonHelpers.h"

#include <asio/yield.hpp>

namespace Modio
{
    namespace Detail
    {
        class DeleteModDependenciesOp
        {
        public:
            DeleteModDependenciesOp(Modio::ModID ModID, const std::vector<Modio::ModID>& Dependencies)
                : ModID(ModID)
            {
                SubmitParams = Modio::Detail::DeleteModDependenciesRequest
                                .SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
                                .SetModID(ModID);

                // Add each dependency to the payload
                std::size_t Index = 0;
                for (const Modio::ModID& Dependency : Dependencies)
                {
                    SubmitParams = SubmitParams.AppendPayloadValue(
                        fmt::format("dependencies[{}]", Index),
                        fmt::format("{}", Dependency));
                    Index++;
                }
            }

            template<typename CoroType>
            void operator()(CoroType& Self, Modio::ErrorCode ec = {})
            {
                reenter(CoroutineState)
                {
                    yield Modio::Detail::PerformRequestAndGetResponseAsync(
                        ResponseBodyBuffer,
                        SubmitParams,
                        Modio::Detail::CachedResponse::Disallow,
                        std::move(Self));

                    Self.complete(ec);
                    return;
                }
            }

        private:
            Modio::ModID ModID;
            Modio::Detail::HttpRequestParams SubmitParams;
            asio::coroutine CoroutineState;
            Modio::Detail::DynamicBuffer ResponseBodyBuffer;
        };
    } // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>