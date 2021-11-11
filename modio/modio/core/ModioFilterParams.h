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

#include "ModioGeneratedVariables.h"

#include "modio/detail/ModioDefines.h"
#include "modio/core/ModioCoreTypes.h"
#include <chrono>
#include <string>
#include <vector>

namespace Modio
{
	/// @docpublic
	/// @brief Class storing a set of filter parameters for use in xref:Modio::ListAllModsAsync[]
	class FilterParams
	{
	public:
		/// @docpublic
		/// @brief Enum indicating which field should be used to sort the results
		enum class SortFieldType
		{
			ID, /** use mod ID (default) */
			DownloadsToday, /** use number of downloads in last 24 (exposed in REST API as 'popular' */
			SubscriberCount, /** use number of subscribers */
			Rating, /** use mod rating */
			DateMarkedLive, /** use date mod was marked live */
			DateUpdated /** use date mod was last updated */
		};

		/// @docpublic
		/// @brief Enum indicating which direction sorting should be applied
		enum class SortDirection
		{
			Ascending, /** (default) */
			Descending
		};

		/// @docpublic
		/// @brief Indicates results should be sorted using the specified field and direction
		/// @param ByField Field to sort with
		/// @param ByDirection Direction to sort
		/// @return *this
		MODIO_IMPL FilterParams& SortBy(SortFieldType ByField, SortDirection ByDirection);

		/// @docpublic
		/// @brief Only include mods where the name contains the provided string
		/// @param SearchString Search string
		/// @return *this
		MODIO_IMPL FilterParams& NameContains(std::string SearchString);

		/// @docpublic
		/// @brief Only include mods where the name contains at least one of the provided strings (string1 OR string2 OR
		/// stringN...)
		/// @tparam ...Args std::string
		/// @param SearchString First search string
		/// @param ...args Additional search strings
		/// @return *this
		MODIO_IMPL FilterParams& NameContains(const std::vector<std::string>& SearchString);

		/// @docpublic
		/// @brief Only include mods with the specified IDs
		/// @param IDSet The list of Mod IDs to filter on
		/// @return *this
		MODIO_IMPL FilterParams& MatchingIDs(const std::vector<Modio::ModID>& IDSet);

		/// @docpublic
		/// @brief Exclude mods with the specified IDs. This takes precedence over <<MatchingIDs>>.
		/// @param IDSet The list of Mod IDs to exclude
		/// @return *this
		MODIO_IMPL FilterParams& ExcludingIDs(const std::vector<Modio::ModID>& IDSet);

		/// @docpublic
		/// @brief Only include mods that were marked live (i.e released) after the specified date
		/// @param LiveAfter Minimum date
		/// @return *this
		MODIO_IMPL FilterParams& MarkedLiveAfter(std::chrono::system_clock::time_point LiveAfter);

		/// @docpublic
		/// @brief Only include mods that were marked live (i.e released) before the specified date
		/// @param LiveBefore Maximum date
		/// @return *this
		MODIO_IMPL FilterParams& MarkedLiveBefore(std::chrono::system_clock::time_point LiveBefore);

		/// @docpublic
		/// @brief Only include mods that have the specified tag
		/// @param Tag Tag to include
		/// @return *this
		MODIO_IMPL FilterParams& WithTags(std::string Tag);

		/// @docpublic
		/// @brief Only include mods that have all the specified tags (tag1 AND tag2 AND tagN...)
		/// @param NewTags The set of tags to filter on
		/// @return *this
		MODIO_IMPL FilterParams& WithTags(std::vector<std::string> NewTags);

		/// @docpublic
		/// @brief Only include mods that do not have the specified tag
		/// @param Tag Tag to exclude
		/// @return *this
		MODIO_IMPL FilterParams& WithoutTags(std::string Tag);

		/// @docpublic
		/// @brief Only include mods that do not have any of the specified tags ( NOT (tag1 OR tag2 OR tagN...))
		/// @param NewTags Tags to exclude
		/// @return *this
		MODIO_IMPL FilterParams& WithoutTags(std::vector<std::string> NewTags);

		/// @docpublic
		/// @brief Only include mods that contain a metadata blob that contains the specified string
		/// @param SearchString String to search for
		/// @return *this
		MODIO_IMPL FilterParams& MetadataLike(std::string SearchString);

		/// @docpublic
		/// @brief Returns a sub-range of query results from StartIndex to StartIndex + ResultCount
		/// @param StartIndex Zero-based index of first result to return
		/// @param ResultCount Number of results to return
		/// @return *this
		MODIO_IMPL FilterParams& IndexedResults(std::size_t StartIndex, std::size_t ResultCount);

		/// @docpublic
		/// @brief Returns a sub-range of query results based on a specified page size and index
		/// @param PageNumber Zero-based index of page to return
		/// @param PageSize Number of results in a page
		/// @return
		MODIO_IMPL FilterParams& PagedResults(std::size_t PageNumber, std::size_t PageSize);

		/// @docpublic
		/// @brief Default FilterParams constructor. Sorts by mod ID in ascending order and returns 100 results (the
		/// maximum allowed by the REST API).
		MODIO_IMPL FilterParams();

		/// @docinternal
		/// @brief Converts the filter params to a string suitable for use in the REST API
		/// @return std::string containing the filter parameters
		MODIO_IMPL std::string ToString() const;

	private:
		MODIO_IMPL FilterParams& AppendValue(std::vector<std::string>& Vector, std::string Tag);

		/*template<typename... Args>
		FilterParams& AppendValue(std::vector<std::string>& Vector, std::string Tag, Args... args)
		{
			AppendValue(Vector, Tag);
			return AppendValue(Vector, args...);
		}*/

		SortFieldType SortField;
		SortDirection Direction;

		std::vector<std::string> SearchKeywords;
		Modio::Optional<std::chrono::system_clock::time_point> DateRangeBegin;
		Modio::Optional<std::chrono::system_clock::time_point> DateRangeEnd;

		std::vector<std::string> Tags;
		std::vector<std::string> ExcludedTags;

		std::vector<Modio::ModID> IncludedIDs;
		std::vector<Modio::ModID> ExcludedIDs;

		Modio::Optional<std::string> MetadataBlobSearchString;

		bool IsPaged;
		std::size_t Index;
		std::size_t Count;
	};

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioFilterParams.ipp"
#endif

#include "modio/detail/ModioUndefs.h"
