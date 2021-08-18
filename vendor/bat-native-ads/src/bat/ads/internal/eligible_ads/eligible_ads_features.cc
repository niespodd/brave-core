/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/eligible_ads/eligible_ads_features.h"

#include <string>

#include "base/metrics/field_trial_params.h"
#include "bat/ads/internal/eligible_ads/eligible_ads_features_util.h"

namespace ads {
namespace features {

namespace {

const char kFeatureName[] = "EligibleAds";
const char kFieldTrialParameterAdFeatureWeights[] = "ad_feature_weights";
const std::vector<double> kDefaultWeights = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

}  // namespace

const base::Feature kEligibleAds{kFeatureName,
                                 base::FEATURE_ENABLED_BY_DEFAULT};

bool IsEligibleAdsEnabled() {
  return base::FeatureList::IsEnabled(kEligibleAds);
}

std::vector<double> GetAdFeatureWeights() {
  std::string param_value = GetFieldTrialParamValueByFeature(
      kEligibleAds, kFieldTrialParameterAdFeatureWeights);

  std::vector<double> weights = ToAdFeatureWeights(param_value);
  if (weights.empty()) {
    weights = kDefaultWeights;
  }

  return weights;
}

}  // namespace features
}  // namespace ads
