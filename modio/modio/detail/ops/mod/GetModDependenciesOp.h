#pragma once
#include "modio/core/ModioBuffer.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/http/ModioHttpParams.h"

#include "asio/yield.hpp"
namespace Modio
{
	namespace Detail
	{
		class GetModDependenciesOp
		{
		public:
			GetModDependenciesOp(Modio::ModID ModID, Modio::GameID GameID) : ModID(ModID), GameID(GameID) {};
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, Modio::Detail::GetModDependenciesRequest.SetGameID(GameID).SetModID(ModID),
						Modio::Detail::CachedResponse::Allow, std::move(Self));
					if (ec)
					{
						Self.complete(ec, {});
						return;
					}
					else
					{
						Modio::Optional<Modio::ModDependencyList> List =
							TryMarshalResponse<Modio::ModDependencyList>(ResponseBodyBuffer);
						if (List.has_value())
						{
							Self.complete({}, List);
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						}
					}
				}
			}

		private:
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			asio::coroutine CoroutineState;
			Modio::ModID ModID;
			Modio::GameID GameID;
		};
	} // namespace Detail
} // namespace Modio
#include "asio/unyield.hpp"