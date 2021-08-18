/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/eligible_ads/eligible_ads_util.h"

#include <vector>

#include "base/guid.h"
#include "base/test/scoped_feature_list.h"
#include "bat/ads/internal/bundle/creative_ad_notification_info.h"
#include "bat/ads/internal/eligible_ads/eligible_ads_features.h"
#include "bat/ads/internal/number_util.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

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

CreativeAdNotificationInfo GetCreativeAdNotificationForIdAndSegment(
    const std::string& creative_instance_id,
    const std::string& segment) {
  CreativeAdNotificationInfo creative_ad_notification;

  creative_ad_notification.creative_instance_id = creative_instance_id;
  creative_ad_notification.creative_set_id = base::GenerateGUID();
  creative_ad_notification.campaign_id = base::GenerateGUID();
  creative_ad_notification.start_at_timestamp = DistantPastAsTimestamp();
  creative_ad_notification.end_at_timestamp = DistantFutureAsTimestamp();
  creative_ad_notification.daily_cap = 1;
  creative_ad_notification.advertiser_id = base::GenerateGUID();
  creative_ad_notification.priority = 1;
  creative_ad_notification.ptr = 1.0;
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

// TODO(Moritz Haller): finish tests (use parameterised tests for templates)

// GroupEligibleAdsByCreativeInstanceId
TEST(BatAdsEligibleAdsUtilTest, GroupEligibleAdsByCreativeInstanceId) {
  // Arrange
  std::vector<CreativeAdNotificationInfo> eligible_ads;

  eligible_ads.push_back(
      GetCreativeAdNotificationForIdAndSegment(base::GenerateGUID(), "foo"));

  const std::string creative_instance_id = base::GenerateGUID();
  const CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotificationForIdAndSegment(creative_instance_id, "foo");
  eligible_ads.push_back(creative_ad_notification);

  eligible_ads.push_back(
      GetCreativeAdNotificationForIdAndSegment(base::GenerateGUID(), "bar"));

  eligible_ads.push_back(
      GetCreativeAdNotificationForIdAndSegment(creative_instance_id, "bar"));

  // Act
  const std::map<std::string, AdPredictorInfo<CreativeAdNotificationInfo>> ads =
      GroupEligibleAdsByCreativeInstanceId(eligible_ads);

  ASSERT_TRUE(!ads.empty());

  // Assert
  AdPredictorInfo<CreativeAdNotificationInfo> expected_ad_predictor;
  expected_ad_predictor.segments = {"foo", "bar"};
  expected_ad_predictor.creative_ad = creative_ad_notification;

  // EXPECT_EQ(expected_ad_predictor, ads.at(creative_instance_id));
}

}  // namespace ads
