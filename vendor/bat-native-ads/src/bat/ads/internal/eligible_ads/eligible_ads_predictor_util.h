/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_ELIGIBLE_ADS_PREDICTOR_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_ELIGIBLE_ADS_PREDICTOR_UTIL_H_

#include <string>
#include <vector>

#include "base/time/time.h"
#include "bat/ads/internal/ad_events/ad_event_info.h"
#include "bat/ads/internal/ad_events/ad_event_util.h"
#include "bat/ads/internal/ad_targeting/ad_targeting_segment_util.h"
#include "bat/ads/internal/container_util.h"
#include "bat/ads/internal/eligible_ads/ad_predictor_info.h"
#include "bat/ads/internal/eligible_ads/eligible_ads_features.h"

namespace ads {

const int kMatchesIntentChildSegmentWeight = 0;
const int kMatchesIntentParentSegmentWeight = 1;
const int kMatchesInterestChildSegmentWeight = 2;
const int kMatchesInterestParentSegmentWeight = 3;
const int kAdLastSeenInHoursWeight = 4;
const int kAdvertiserLastSeenInHoursWeight = 5;
const int kPriorityWeight = 6;

template <typename T>
AdPredictorInfo<T> ComputePredictorFeatures(
    const AdPredictorInfo<T>& ad_predictor,
    const AdEventList& ad_events,
    const SegmentList& interest_segments,
    const SegmentList& intent_segments) {
  AdPredictorInfo<T> mutable_ad_predictor;

  const SegmentList intent_child_segments_intersection =
      SetIntersection(intent_segments, ad_predictor.segments);
  mutable_ad_predictor.does_match_intent_child_segments =
      intent_child_segments_intersection.empty() ? false : true;

  const SegmentList parent_intent_segments =
      GetParentSegments(interest_segments);
  const SegmentList intent_parent_segments_intersection =
      SetIntersection(parent_intent_segments, ad_predictor.segments);
  mutable_ad_predictor.does_match_intent_parent_segments =
      intent_parent_segments_intersection.empty() ? false : true;

  const SegmentList interest_child_segments_intersection =
      SetIntersection(interest_segments, ad_predictor.segments);
  mutable_ad_predictor.does_match_interest_child_segments =
      interest_child_segments_intersection.empty() ? false : true;

  const SegmentList parent_interest_segments =
      GetParentSegments(interest_segments);
  const SegmentList interest_parent_segments_intersection =
      SetIntersection(parent_interest_segments, ad_predictor.segments);
  mutable_ad_predictor.does_match_interest_parent_segments =
      interest_parent_segments_intersection.empty() ? false : true;

  const base::Time now = base::Time::Now();

  const base::Time last_seen_ad =
      GetLastSeenAdTime(ad_events, ad_predictor.creative_ad);
  mutable_ad_predictor.ad_last_seen_hours_ago = (now - last_seen_ad).InHours();

  const base::Time last_seen_advertiser =
      GetLastSeenAdvertiserTime(ad_events, ad_predictor.creative_ad);
  mutable_ad_predictor.advertiser_last_seen_hours_ago =
      (now - last_seen_advertiser).InHours();

  return mutable_ad_predictor;
}

template <typename T>
double ComputePredictorScore(const AdPredictorInfo<T>& ad_predictor) {
  std::vector<double> weights = features::GetAdFeatureWeights();

  double score = 0.0;

  if (ad_predictor.does_match_intent_child_segments) {
    score += weights.at(kMatchesIntentChildSegmentWeight);
  } else if (ad_predictor.does_match_intent_parent_segments) {
    score += weights.at(kMatchesIntentParentSegmentWeight);
  }

  if (ad_predictor.does_match_interest_child_segments) {
    score += weights.at(kMatchesInterestChildSegmentWeight);
  } else if (ad_predictor.does_match_interest_parent_segments) {
    score += weights.at(kMatchesInterestParentSegmentWeight);
  }

  if (ad_predictor.ad_last_seen_hours_ago <= base::Time::kHoursPerDay) {
    score += weights.at(kAdLastSeenInHoursWeight) *
             ad_predictor.ad_last_seen_hours_ago /
             static_cast<double>(base::Time::kHoursPerDay);
  }

  if (ad_predictor.advertiser_last_seen_hours_ago <= base::Time::kHoursPerDay) {
    score += weights.at(kAdvertiserLastSeenInHoursWeight) *
             ad_predictor.advertiser_last_seen_hours_ago /
             static_cast<double>(base::Time::kHoursPerDay);
  }

  if (ad_predictor.creative_ad.priority > 0) {
    score += weights.at(kPriorityWeight) / ad_predictor.creative_ad.priority;
  }

  score *= ad_predictor.creative_ad.ptr;

  return score;
}

template <typename T>
std::map<std::string, AdPredictorInfo<T>> ComputePredictorFeaturesAndScores(
    const std::map<std::string, AdPredictorInfo<T>>& ads,
    const AdEventList& ad_events,
    const SegmentList& interest_segments,
    const SegmentList& intent_segments) {
  std::map<std::string, AdPredictorInfo<T>> ads_with_features;

  for (auto& ad : ads) {
    // TODO(Moritz Haller): check if works with copy constructor
    // AdPredictorInfo<T> ad_predictor = ad.second;
    AdPredictorInfo<T> ad_predictor = ComputePredictorFeatures(
        ad.second, ad_events, interest_segments, intent_segments);
    ad_predictor.score = ComputePredictorScore(ad_predictor);
    ads_with_features.insert(
        {ad_predictor.creative_ad.creative_instance_id, ad_predictor});
  }

  return ads_with_features;
}

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_ELIGIBLE_ADS_PREDICTOR_UTIL_H_
