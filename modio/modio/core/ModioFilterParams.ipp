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
	#include "modio/core/ModioFilterParams.h"
#endif

#include "modio/core/entities/ModioProfileMaturity.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ModioFormatters.h"

namespace Modio
{
	Modio::FilterParams& FilterParams::SortBy(SortFieldType ByField, SortDirection ByDirection)
	{
		SortField = ByField;
		Direction = ByDirection;
		return *this;
	}

	Modio::FilterParams& FilterParams::NameContains(const std::vector<std::string>& SearchString)
	{
		// @todo: NameContains overloads behave differently. Take single arguments won't append empty values but this
		// version will same seem to be correct of different functions
		if (SearchString.size())
		{
			SearchKeywords.clear();
			SearchKeywords.insert(SearchKeywords.end(), SearchString.begin(), SearchString.end());
		}
		return *this;
	}

	Modio::FilterParams& FilterParams::NameContains(std::string SearchString)
	{
		if (SearchString.size())
		{
			SearchKeywords.clear();
			SearchKeywords.push_back(SearchString);
		}
		return *this;
	}

	Modio::FilterParams& FilterParams::MatchingAuthor(const Modio::UserID& UserId)
	{
		AuthorUserIds.clear();
		AuthorUserIds.push_back(UserId);
		return *this;
	}

	Modio::FilterParams& FilterParams::MatchingAuthors(const std::vector<Modio::UserID>& UserIds)
	{
		AuthorUserIds = UserIds;
		return *this;
	}

	Modio::FilterParams& FilterParams::MatchingIDs(const std::vector<Modio::ModID>& IDSet)
	{
		IncludedIDs = IDSet;
		return *this;
	}

	Modio::FilterParams& FilterParams::ExcludingIDs(const std::vector<Modio::ModID>& IDSet)
	{
		ExcludedIDs = IDSet;
		return *this;
	}

	Modio::FilterParams& FilterParams::MarkedLiveAfter(std::chrono::system_clock::time_point LiveAfter)
	{
		DateRangeBegin = LiveAfter;
		return *this;
	}

	Modio::FilterParams& FilterParams::MarkedLiveBefore(std::chrono::system_clock::time_point LiveBefore)
	{
		DateRangeEnd = LiveBefore;
		return *this;
	}

	Modio::FilterParams& FilterParams::WithTags(std::vector<std::string> NewTags)
	{
		Tags = std::move(NewTags);
		return *this;
	}

	Modio::FilterParams& FilterParams::WithTags(std::string Tag)
	{
		Tags.clear();
		Tags.push_back(Tag);
		return *this;
	}

	Modio::FilterParams& FilterParams::WithoutTags(std::vector<std::string> NewTags)
	{
		ExcludedTags = std::move(NewTags);
		return *this;
	}

	Modio::FilterParams& FilterParams::WithoutTags(std::string Tag)
	{
		ExcludedTags.clear();
		ExcludedTags.push_back(Tag);
		return *this;
	}

	Modio::FilterParams& Modio::FilterParams::MetadataLike(std::string SearchString)
	{
		MetadataBlobSearchString = SearchString;
		return *this;
	}

	Modio::FilterParams& FilterParams::IndexedResults(std::size_t StartIndex, std::size_t ResultCount)
	{
		IsPaged = false;
		Index = std::max(std::size_t(0), StartIndex);
		Count = std::max(std::size_t(0), ResultCount);
		return *this;
	}

	Modio::FilterParams& FilterParams::PagedResults(std::size_t PageNumber, std::size_t PageSize)
	{
		IsPaged = true;
		Index = std::max(std::size_t(0), PageNumber);
		Count = std::max(std::size_t(0), PageSize);
		return *this;
	}

	Modio::FilterParams& FilterParams::RevenueType(RevenueFilterType ByRevenue)
	{
		Revenue = ByRevenue;
		return *this;
	}

	Modio::FilterParams& FilterParams::DisallowMatureContent()
	{
		Maturity = Modio::MaturityOption::None;
		return *this;
	}

	Modio::FilterParams& FilterParams::WithMatureContentFlags(Modio::MaturityOption ByMaturity)
	{
		Maturity = ByMaturity;
		return *this;
	}

	FilterParams::FilterParams()
		: SortField(SortFieldType::ID),
		  Direction(SortDirection::Ascending),
		  IsPaged(false),
		  Index(0),
		  Count(100)
	{}

	std::map<std::string, std::string> Modio::FilterParams::ToQueryParamaters() const
	{
		std::map<std::string, std::string> FilterFields;
		std::string SortStr;

		// The sorts listed at https://docs.mod.io/restapiref/#get-mods is inverted for some reason compared to the explanation at
		// https://docs.mod.io/restapiref/#sorting
		bool bInvertedSort = false;

		switch (SortField)
		{
			case SortFieldType::DateMarkedLive:
				SortStr = "date_live";
				break;
			case SortFieldType::DateUpdated:
				SortStr = "date_updated";
				break;
			case SortFieldType::DownloadsToday:
				SortStr = "popular";
				break;
			case SortFieldType::Rating:
				SortStr = "rating";
				bInvertedSort = true;
				break;
			case SortFieldType::SubscriberCount:
				SortStr = "subscribers";
				bInvertedSort = true;
				break;
			case SortFieldType::ID:
				SortStr = "id";
				break;
			case SortFieldType::DownloadsTotal:
				SortStr = "downloads";
				bInvertedSort = true;
				break;
			case SortFieldType::Alphabetical:
				SortStr = "name";
				break;

			default:
				break;
		}

		FilterFields.emplace("_sort", fmt::format("{}{}",
												  ((Direction == SortDirection::Descending && !bInvertedSort) ||
														   (Direction == SortDirection::Ascending && bInvertedSort)
													   ? "-"
													   : ""),
												  SortStr));

		if (!SearchKeywords.empty())
		{
			std::string SearchStr;
			for (std::string Keyword : SearchKeywords)
			{
				SearchStr += Keyword + " ";
			}
			SearchStr.resize(SearchStr.size() - 1);
			FilterFields.emplace("_q", SearchStr);
		}

		if (!AuthorUserIds.empty())
		{
			FilterFields.emplace("submitted_by-in", fmt::format("{}", fmt::join(AuthorUserIds, ",")));
		}

		if (DateRangeBegin)
		{
			// The mod.io API expects an integer in seconds
			FilterFields.emplace(
				"date_live-min",
				fmt::format(
					"{}",
					std::chrono::duration_cast<std::chrono::seconds>(DateRangeBegin->time_since_epoch()).count()));
		}

		if (DateRangeEnd)
		{
			// The mod.io API expects an integer in seconds
			FilterFields.emplace(
				"date_live-max",
				fmt::format(
					"{}", std::chrono::duration_cast<std::chrono::seconds>(DateRangeEnd->time_since_epoch()).count()));
		}

		if (!Tags.empty())
		{
			std::string TagStr;
			for (std::string Tag : Tags)
			{
				TagStr += Tag + ",";
			}
			TagStr.resize(TagStr.size() - 1);
			FilterFields.emplace("tags", TagStr);
		}

		if (!ExcludedTags.empty())
		{
			std::string ExcludedTagStr;
			for (std::string Tag : ExcludedTags)
			{
				ExcludedTagStr += Tag + ",";
			}
			ExcludedTagStr.resize(ExcludedTagStr.size() - 1);
			FilterFields.emplace("tags-not-in", ExcludedTagStr);
		}

		if (MetadataBlobSearchString)
		{
			FilterFields.emplace("metadata_blob-lk", fmt::format("*{}*", *MetadataBlobSearchString));
		}

		std::string ResultLimitStr;
		if (IsPaged)
		{
			FilterFields.emplace("_limit", std::to_string(Count));
			FilterFields.emplace("_offset", std::to_string(Count * Index));
		}
		else
		{
			FilterFields.emplace("_limit", std::to_string(Count));
			FilterFields.emplace("_offset", std::to_string(Index));
		}

		if (!IncludedIDs.empty())
		{
			FilterFields.emplace("id-in", fmt::format("{}", fmt::join(IncludedIDs, ",")));
		}

		if (!ExcludedIDs.empty())
		{
			FilterFields.emplace("id-not-in", fmt::format("{}", fmt::join(ExcludedIDs, ",")));
		}

		if (Revenue.has_value())
		{
			switch (Revenue.value())
			{
				case RevenueFilterType::Free:
					FilterFields.emplace("revenue_type", "0");
					break;
				case RevenueFilterType::Paid:
					FilterFields.emplace("revenue_type", "1");
					break;
				case RevenueFilterType::FreeAndPaid:
					FilterFields.emplace("revenue_type", "2");
					break;
				default:;
			}
		}

		if (Maturity.has_value())
		{
			auto maturityOption = static_cast<uint8_t>(Maturity.value());
			FilterFields.emplace("maturity_option", std::to_string(maturityOption));
		}

		return FilterFields;
	}

	Modio::FilterParams& FilterParams::AppendValue(std::vector<std::string>& MODIO_UNUSED_ARGUMENT(Vector),
												   std::string Tag)
	{
		Tags.push_back(Tag);
		return *this;
	}

} // namespace Modio
