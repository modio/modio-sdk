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
#include <nlohmann/json.hpp>
#include <string>
namespace Modio
{
	/// @docpublic
	/// @brief Contains download stats and ratings for a mod
	struct ModStats
	{
		/// @brief Current rank of the mod.
		std::int64_t PopularityRankPosition = 0;
		/// @brief Number of ranking spots the current rank is measured against.
		std::int64_t PopularityRankTotalMods = 0;
		/// @brief Number of total mod downloads.
		std::int64_t DownloadsTotal = 0;
		/// @brief Number of total users who have subscribed to the mod.
		std::int64_t SubscribersTotal = 0;
		/// @brief Number of times this mod has been rated.
		std::int64_t RatingTotal = 0;
		/// @brief Number of positive ratings.
		std::int64_t RatingPositive = 0;
		/// @brief Number of negative ratings.
		std::int64_t RatingNegative = 0;
		/// @brief Number of positive ratings, divided by the total ratings to determine it’s percentage score.
		std::int64_t RatingPercentagePositive = 0;
		/// @brief Overall rating of this item calculated using the [Wilson score confidence
		/// interval](https://www.evanmiller.org/how-not-to-sort-by-average-Ratinghtml). This column is good to sort
		/// on, as it will order items based on number of ratings and will place items with many positive ratings
		/// above those with a higher score but fewer ratings.
		double RatingWeightedAggregate = 0;
		/// @brief Textual representation of the rating in format: Overwhelmingly
		/// Positive -> Very Positive -> Positive -> Mostly Positive -> Mixed ->
		/// Negative -> Mostly Negative -> Very Negative -> Overwhelmingly
		/// Negative -> Unrated
		std::string RatingDisplayText = "";

		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::ModStats& ModStats);
		friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::ModStats Stats);

		friend bool operator==(const Modio::ModStats& A, const Modio::ModStats& B)
		{
			if ((A.PopularityRankPosition == B.PopularityRankPosition) &&
				(A.PopularityRankTotalMods == B.PopularityRankTotalMods) && (A.DownloadsTotal == B.DownloadsTotal) &&
				(A.SubscribersTotal == B.SubscribersTotal) && (A.RatingTotal == B.RatingTotal) &&
				(A.RatingPositive == B.RatingPositive) && (A.RatingNegative == B.RatingNegative) &&
				(A.RatingPercentagePositive == B.RatingPercentagePositive) &&
				(A.RatingWeightedAggregate == B.RatingWeightedAggregate) &&
				(A.RatingDisplayText == B.RatingDisplayText))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioModStats.ipp"
#endif
