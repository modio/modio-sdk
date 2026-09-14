#pragma once

#include <utility>

#include "modio/core/ModioFilterParams.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

// FilterParams is a native-only type and never returned from the REST API, however some of the values it contains are
// returned by the backend in other places, so those values have serialization implementations here

namespace Modio
{
	NLOHMANN_JSON_SERIALIZE_ENUM(
		Modio::FilterParams::Timespan,
		{{Modio::FilterParams::Timespan::Today, Modio::Detail::Constants::TimespanKeys::Today},
		 {Modio::FilterParams::Timespan::Week, Modio::Detail::Constants::TimespanKeys::Week},
		 {Modio::FilterParams::Timespan::Month, Modio::Detail::Constants::TimespanKeys::Month},
		 {Modio::FilterParams::Timespan::ThreeMonths, Modio::Detail::Constants::TimespanKeys::ThreeMonths},
		 {Modio::FilterParams::Timespan::Year, Modio::Detail::Constants::TimespanKeys::Year},
		 {Modio::FilterParams::Timespan::AllTime, Modio::Detail::Constants::TimespanKeys::AllTime}})

	NLOHMANN_JSON_SERIALIZE_ENUM(Modio::FilterParams::RevenueFilterType,
								 {{Modio::FilterParams::RevenueFilterType::Free,
								   static_cast<std::underlying_type_t<Modio::FilterParams::RevenueFilterType>>(
									   Modio::FilterParams::RevenueFilterType::Free)},
								  {Modio::FilterParams::RevenueFilterType::Paid,
								   static_cast<std::underlying_type_t<Modio::FilterParams::RevenueFilterType>>(
									   Modio::FilterParams::RevenueFilterType::Paid)},
								  {Modio::FilterParams::RevenueFilterType::FreeAndPaid,
								   static_cast<std::underlying_type_t<Modio::FilterParams::RevenueFilterType>>(
									   Modio::FilterParams::RevenueFilterType::FreeAndPaid)},
								  {Modio::FilterParams::RevenueFilterType::FreeAndPaidWeighted,
								   static_cast<std::underlying_type_t<Modio::FilterParams::RevenueFilterType>>(
									   Modio::FilterParams::RevenueFilterType::FreeAndPaidWeighted)}})
} // namespace Modio