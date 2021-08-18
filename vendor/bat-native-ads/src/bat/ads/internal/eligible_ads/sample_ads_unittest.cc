/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/eligible_ads/sample_ads.h"

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

// CalculateNormalisingConstant
TEST(BatAdsEligibleAdsUtilTest, CalculateNormalisingConstant) {
  // Arrange
  std::map<std::string, AdPredictorInfo<CreativeAdNotificationInfo>> ads;
  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_1;
  ad_predictor_1.score = 1.1;
  ads[base::GenerateGUID()] = ad_predictor_1;

  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_2;
  ad_predictor_2.score = 2.2;
  ads[base::GenerateGUID()] = ad_predictor_2;

  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_3;
  ad_predictor_3.score = 3.3;
  ads[base::GenerateGUID()] = ad_predictor_3;

  // Act
  const double normalising_constant = CalculateNormalisingConstant(ads);

  // Assert
  const double expected_normalising_constant = 6.6;
  EXPECT_TRUE(
      DoubleEquals(expected_normalising_constant, normalising_constant));
}

// SampleFromAds
TEST(BatAdsEligibleAdsUtilTest, SampleFromAdsWithZeroScores) {
  // Arrange
  std::map<std::string, AdPredictorInfo<CreativeAdNotificationInfo>> ads;
  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_1;
  ad_predictor_1.score = 0;
  ads[base::GenerateGUID()] = ad_predictor_1;

  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_2;
  ad_predictor_2.score = 0;
  ads[base::GenerateGUID()] = ad_predictor_2;

  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_3;
  ad_predictor_3.score = 0;
  ads[base::GenerateGUID()] = ad_predictor_3;

  // Act
  const absl::optional<CreativeAdNotificationInfo> ad = SampleFromAds(ads);

  // Assert
  const absl::optional<CreativeAdNotificationInfo> expected_ad = absl::nullopt;
  EXPECT_EQ(expected_ad, ad);
}

TEST(BatAdsEligibleAdsUtilTest,
     DeterministicallySampleFromAdsWithOneNonZeroScore) {
  // Arrange
  std::map<std::string, AdPredictorInfo<CreativeAdNotificationInfo>> ads;

  CreativeAdNotificationInfo creative_ad_notification_1 =
      GetCreativeAdNotification("foo-bar", 1.0, 1);
  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_1;
  ad_predictor_1.score = 0;
  ad_predictor_1.creative_ad = creative_ad_notification_1;
  ads[creative_ad_notification_1.creative_instance_id] = ad_predictor_1;

  CreativeAdNotificationInfo creative_ad_notification_2 =
      GetCreativeAdNotification("foo-bar", 1.0, 1);
  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_2;
  ad_predictor_2.score = 0.1;
  ad_predictor_2.creative_ad = creative_ad_notification_2;
  ads[creative_ad_notification_2.creative_instance_id] = ad_predictor_2;

  CreativeAdNotificationInfo creative_ad_notification_3 =
      GetCreativeAdNotification("foo-bar", 1.0, 1);
  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_3;
  ad_predictor_3.score = 0;
  ad_predictor_3.creative_ad = creative_ad_notification_3;
  ads[creative_ad_notification_3.creative_instance_id] = ad_predictor_3;

  // Act
  for (int i = 0; i < 10; i++) {
    CreativeAdNotificationInfo expected_ad = creative_ad_notification_2;
    const absl::optional<CreativeAdNotificationInfo> ad = SampleFromAds(ads);
    EXPECT_EQ(expected_ad, ad.value());
  }

  // Assert
}

TEST(BatAdsEligibleAdsUtilTest, ProbabilisticallySampleFromAds) {
  // Arrange
  std::map<std::string, AdPredictorInfo<CreativeAdNotificationInfo>> ads;

  CreativeAdNotificationInfo creative_ad_notification_1 =
      GetCreativeAdNotification("foo-bar", 1.0, 1);
  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_1;
  ad_predictor_1.creative_ad = creative_ad_notification_1;
  ad_predictor_1.score = 3;
  ads[creative_ad_notification_1.creative_instance_id] = ad_predictor_1;

  CreativeAdNotificationInfo creative_ad_notification_2 =
      GetCreativeAdNotification("foo-bar", 1.0, 1);
  AdPredictorInfo<CreativeAdNotificationInfo> ad_predictor_2;
  ad_predictor_2.creative_ad = creative_ad_notification_2;
  ad_predictor_2.score = 3;
  ads[creative_ad_notification_2.creative_instance_id] = ad_predictor_2;

  // Act
  int ads_1_count = 0;
  int ads_2_count = 0;

  // P(X>1) > 0.99999999 with X~Bin(n=25, p=0.5), i.e. less than 1 in 100M tests
  // are expected to fail
  for (int i = 0; i < 25; i++) {
    const absl::optional<CreativeAdNotificationInfo> ad = SampleFromAds(ads);

    if (ad.value().creative_instance_id ==
        creative_ad_notification_1.creative_instance_id) {
      ads_1_count += 1;
    } else if (ad.value().creative_instance_id ==
               creative_ad_notification_2.creative_instance_id) {
      ads_2_count += 1;
    }
  }

  // Assert
  EXPECT_FALSE(ads_1_count == 0 || ads_2_count == 0);
}

}  // namespace ads
