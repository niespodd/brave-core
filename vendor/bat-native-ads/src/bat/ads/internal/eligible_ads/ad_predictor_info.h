/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_AD_PREDICTOR_INFO
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_AD_PREDICTOR_INFO

#include "bat/ads/internal/ad_targeting/ad_targeting_segment.h"

namespace ads {

template <typename T>
struct AdPredictorInfo {
  AdPredictorInfo();
  AdPredictorInfo(const AdPredictorInfo& info);
  ~AdPredictorInfo();

  T creative_ad;
  SegmentList segments;
  bool does_match_intent_child_segments;
  bool does_match_intent_parent_segments;
  bool does_match_interest_child_segments;
  bool does_match_interest_parent_segments;
  int ad_last_seen_hours_ago = 0;
  int advertiser_last_seen_hours_ago = 0;
  double score = 0.0;
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_AD_PREDICTOR_INFO
