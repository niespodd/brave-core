/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/eligible_ads/eligible_ads_predictor_util.h"

#include <vector>

#include "base/guid.h"
#include "base/test/scoped_feature_list.h"
#include "bat/ads/internal/bundle/creative_ad_notification_info.h"
#include "bat/ads/internal/eligible_ads/eligible_ads_features.h"
#include "bat/ads/internal/number_util.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

namespace {

CreativeAdNotificationInfo GetCreativeAdNotification(const std::string& segment,
                                                     const double ptr,
                                                     const int priority) {
  CreativeAdNotificationInfo creative_ad_notification;

  creative_ad_notification.creative_instance_id = base::GenerateGUID();
  creative_ad_notification.creative_set_id = base::GenerateGUID();
  creative_ad_notification.campaign_id = base::GenerateGUID();
  creative_ad_notification.start_at_timestamp = DistantPastAsTimestamp();
  creative_ad_notification.end_at_timestamp = DistantFutureAsTimestamp();
  creative_ad_notification.daily_cap = 1;
  creative_ad_notification.advertiser_id = base::GenerateGUID();
  creative_ad_notification.priority = priority;
  creative_ad_notification.ptr = ptr;
  creative_ad_notification.per_day = 1;
  creative_ad_notification.per_week = 1;
  creative_ad_notification.per_month = 1;
  creative_ad_notification.total_max = 1;
  creative_ad_notification.segment = segment;
  creative_ad_notification.geo_targets = {"US"};
  creative_ad_notification.target_url = "https://brave.com";
  CreativeDaypartInfo daypart;
  creative_ad_notification.dayparts = {daypart};
  creative_ad_notification.title = "Test Ad Title";
  creative_ad_notification.body = "Test Ad Body";

  return creative_ad_notification;
}

}  // namespace

// ComputePredictorScore
TEST(BatAdsEligibleAdsUtilTest,
     ComputePredictorScoreWithZeroWeightsNotAllowedByGriffin) {
  // Arrange
  const char kAdFeatureWeights[] = "ad_feature_weights";
  std::map<std::string, std::string> kEligibleAdsParameters;
  kEligibleAdsParameters[kAdFeatureWeights] =
      "0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0";

  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitWithFeaturesAndParameters(
      {{features::kEligibleAds, kEligibleAdsParameters}}, {});

  const std::string segment = "foo-bar";
  const double ptr = 1.0;
  const double priority = 1;
  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotification(segment, ptr, priority);

  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor;
  ad_predictor.creative_ad = creative_ad_notification;
  ad_predictor.segments = {segment};
  ad_predictor.does_match_intent_child_segments = 1;
  ad_predictor.does_match_intent_parent_segments = 0;
  ad_predictor.does_match_interest_child_segments = 0;
  ad_predictor.does_match_interest_parent_segments = 0;
  ad_predictor.ad_last_seen_hours_ago = 15;
  ad_predictor.advertiser_last_seen_hours_ago = 48;

  // Act
  ad_predictor.score = ComputePredictorScore(ad_predictor);

  // Assert
  EXPECT_LT(0, ad_predictor.score);
}

TEST(BatAdsEligibleAdsUtilTest, ComputePredictorScoreWithDefaultWeights) {
  // Arrange
  const std::string segment = "foo-bar";
  const double ptr = 1.0;
  const double priority = 1;
  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotification(segment, ptr, priority);

  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor;
  ad_predictor.creative_ad = creative_ad_notification;
  ad_predictor.segments = {segment};
  ad_predictor.does_match_intent_child_segments = 1;
  ad_predictor.does_match_intent_parent_segments = 0;
  ad_predictor.does_match_interest_child_segments = 0;
  ad_predictor.does_match_interest_parent_segments = 0;
  ad_predictor.ad_last_seen_hours_ago = 15;
  ad_predictor.advertiser_last_seen_hours_ago = 48;

  // Act
  ad_predictor.score = ComputePredictorScore(ad_predictor);

  // Assert
  const double expected_score = 0.0 + 1.0 + 1.0 * (15 / 24.0) + 1.0 * 1.0;
  EXPECT_EQ(expected_score, ad_predictor.score);
}

TEST(BatAdsEligibleAdsUtilTest, ComputePredictorScoreWithEmptyAdFeatures) {
  // Arrange
  const std::string segment = "foo-bar";
  const double ptr = 1.0;
  const double priority = 1;
  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotification(segment, ptr, priority);

  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor;

  // Act
  ad_predictor.score = ComputePredictorScore(ad_predictor);

  // Assert
  const double expected_score = 0;
  EXPECT_EQ(expected_score, ad_predictor.score);
}

// ComputePredictorFeaturesAndScores

}  // namespace ads
