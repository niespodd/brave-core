/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_SAMPLE_ADS_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_SAMPLE_ADS_H_

#include <map>
#include <string>

#include "base/rand_util.h"
#include "bat/ads/internal/eligible_ads/ad_predictor_info.h"
#include "bat/ads/internal/number_util.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace ads {

template <typename T>
double CalculateNormalisingConstant(
    const std::map<std::string, AdPredictorInfo<T>>& ads) {
  double normalising_constant = 0.0;
  for (const auto& ad : ads) {
    normalising_constant += ad.second.score;
  }

  return normalising_constant;
}

template <typename T>
absl::optional<T> SampleFromAds(
    const std::map<std::string, AdPredictorInfo<T>>& ads) {
  const double normalising_constant = CalculateNormalisingConstant(ads);
  if (DoubleIsLessEqual(normalising_constant, 0)) {
    return absl::nullopt;
  }

  const double rand = base::RandDouble();
  double sum = 0;
  T selected_ad;

  for (const auto& ad : ads) {
    const double probability = ad.second.score / normalising_constant;
    sum += probability;
    if (DoubleIsLess(rand, sum)) {
      return ad.second.creative_ad;
    }
  }

  return absl::nullopt;
}

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_SAMPLE_ADS_H_
