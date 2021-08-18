/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/eligible_ads/ad_notifications/eligible_ad_notifications.h"

#include <map>
#include <memory>
#include <string>

#include "base/guid.h"
#include "base/test/scoped_feature_list.h"
#include "bat/ads/internal/ad_events/ad_events.h"
#include "bat/ads/internal/ad_serving/ad_targeting/geographic/subdivision/subdivision_targeting.h"
#include "bat/ads/internal/database/tables/creative_ad_notifications_database_table.h"
#include "bat/ads/internal/eligible_ads/eligible_ads_features.h"
#include "bat/ads/internal/resources/frequency_capping/anti_targeting_resource.h"
#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

class BatAdsEligibleAdNotificationsTest : public UnitTestBase {
 protected:
  BatAdsEligibleAdNotificationsTest()
      : creative_ad_notifications_table_(
            std::make_unique<database::table::CreativeAdNotifications>()) {}

  ~BatAdsEligibleAdNotificationsTest() override = default;

  CreativeAdNotificationInfo GetCreativeAdNotificationForSegment(
      const std::string& segment) {
    CreativeAdNotificationInfo creative_ad_notification;

    creative_ad_notification.creative_instance_id = base::GenerateGUID();
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

  void Save(const CreativeAdNotificationList& creative_ad_notifications) {
    creative_ad_notifications_table_->Save(
        creative_ad_notifications,
        [](const bool success) { ASSERT_TRUE(success); });
  }

  void Log(const CreativeAdNotificationInfo& creative_ad_notification,
           int hours_ago) {
    AdEventInfo ad_event;
    ad_event.uuid = base::GenerateGUID();
    ad_event.type = AdType::kAdNotification;
    ad_event.confirmation_type = ConfirmationType::kViewed;
    ad_event.campaign_id = creative_ad_notification.campaign_id;
    ad_event.creative_set_id = creative_ad_notification.creative_set_id;
    ad_event.creative_instance_id =
        creative_ad_notification.creative_instance_id;
    ad_event.advertiser_id = creative_ad_notification.advertiser_id;
    base::Time timestamp =
        base::Time::Now() - base::TimeDelta::FromHours(hours_ago);
    ad_event.timestamp = static_cast<int64_t>(timestamp.ToDoubleT());

    LogAdEvent(ad_event, [](const bool success) { ASSERT_TRUE(success); });
  }

  std::unique_ptr<database::table::CreativeAdNotifications>
      creative_ad_notifications_table_;
};

TEST_F(BatAdsEligibleAdNotificationsTest, GetAdsForParentChildSegment) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification_1 =
      GetCreativeAdNotificationForSegment("technology & computing");
  creative_ad_notifications.push_back(creative_ad_notification_1);

  CreativeAdNotificationInfo creative_ad_notification_2 =
      GetCreativeAdNotificationForSegment("technology & computing-software");
  creative_ad_notifications.push_back(creative_ad_notification_2);

  Save(creative_ad_notifications);

  // Act
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::EligibleAds eligible_ads(&subdivision_targeting,
                                             &anti_targeting_resource);

  const CreativeAdNotificationList expected_creative_ad_notifications = {
      creative_ad_notification_2};

  eligible_ads.GetForSegments(
      {"technology & computing-software"},
      [&expected_creative_ad_notifications](
          const bool success,
          const CreativeAdNotificationList& creative_ad_notifications) {
        EXPECT_EQ(expected_creative_ad_notifications,
                  creative_ad_notifications);
      });

  // Assert
}

TEST_F(BatAdsEligibleAdNotificationsTest, GetAdsForParentSegment) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotificationForSegment("technology & computing");
  creative_ad_notifications.push_back(creative_ad_notification);

  Save(creative_ad_notifications);

  // Act
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::EligibleAds eligible_ads(&subdivision_targeting,
                                             &anti_targeting_resource);

  const CreativeAdNotificationList expected_creative_ad_notifications = {
      creative_ad_notification};

  eligible_ads.GetForSegments(
      {"technology & computing-software"},
      [&expected_creative_ad_notifications](
          const bool success,
          const CreativeAdNotificationList& creative_ad_notifications) {
        EXPECT_EQ(expected_creative_ad_notifications,
                  creative_ad_notifications);
      });

  // Assert
}

TEST_F(BatAdsEligibleAdNotificationsTest, GetAdsForUntargetedSegment) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotificationForSegment("untargeted");
  creative_ad_notifications.push_back(creative_ad_notification);

  Save(creative_ad_notifications);

  // Act
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::EligibleAds eligible_ads(&subdivision_targeting,
                                             &anti_targeting_resource);

  const CreativeAdNotificationList expected_creative_ad_notifications = {
      creative_ad_notification};

  eligible_ads.GetForSegments(
      {"finance-banking"},
      [&expected_creative_ad_notifications](
          const bool success,
          const CreativeAdNotificationList& creative_ad_notifications) {
        EXPECT_EQ(expected_creative_ad_notifications,
                  creative_ad_notifications);
      });

  // Assert
}

TEST_F(BatAdsEligibleAdNotificationsTest, GetAdsForMultipleSegments) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification_1 =
      GetCreativeAdNotificationForSegment("technology & computing");
  creative_ad_notifications.push_back(creative_ad_notification_1);

  CreativeAdNotificationInfo creative_ad_notification_2 =
      GetCreativeAdNotificationForSegment("finance-banking");
  creative_ad_notifications.push_back(creative_ad_notification_2);

  CreativeAdNotificationInfo creative_ad_notification_3 =
      GetCreativeAdNotificationForSegment("food & drink");
  creative_ad_notifications.push_back(creative_ad_notification_3);

  Save(creative_ad_notifications);

  // Act
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::EligibleAds eligible_ads(&subdivision_targeting,
                                             &anti_targeting_resource);

  const CreativeAdNotificationList expected_creative_ad_notifications = {
      creative_ad_notification_1, creative_ad_notification_2};

  eligible_ads.GetForSegments(
      {"technology & computing", "food & drink"},
      [&expected_creative_ad_notifications](
          const bool success,
          const CreativeAdNotificationList& creative_ad_notifications) {
        EXPECT_EQ(expected_creative_ad_notifications,
                  creative_ad_notifications);
      });

  // Assert
}

TEST_F(BatAdsEligibleAdNotificationsTest, GetAdsForNoSegments) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotificationForSegment("untargeted");
  creative_ad_notifications.push_back(creative_ad_notification);

  Save(creative_ad_notifications);

  // Act
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::EligibleAds eligible_ads(&subdivision_targeting,
                                             &anti_targeting_resource);

  const CreativeAdNotificationList expected_creative_ad_notifications = {
      creative_ad_notification};

  eligible_ads.GetForSegments(
      {}, [&expected_creative_ad_notifications](
              const bool success,
              const CreativeAdNotificationList& creative_ad_notifications) {
        EXPECT_EQ(expected_creative_ad_notifications,
                  creative_ad_notifications);
      });

  // Assert
}

TEST_F(BatAdsEligibleAdNotificationsTest, GetAdsForUnmatchedSegments) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotificationForSegment("technology & computing");
  creative_ad_notifications.push_back(creative_ad_notification);

  Save(creative_ad_notifications);

  // Act
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::EligibleAds eligible_ads(&subdivision_targeting,
                                             &anti_targeting_resource);

  const CreativeAdNotificationList expected_creative_ad_notifications = {};

  eligible_ads.GetForSegments(
      {"UNMATCHED"},
      [&expected_creative_ad_notifications](
          const bool success,
          const CreativeAdNotificationList& creative_ad_notifications) {
        EXPECT_EQ(expected_creative_ad_notifications,
                  creative_ad_notifications);
      });

  // Assert
}

TEST_F(BatAdsEligibleAdNotificationsTest, GetForFeaturesWithoutAds) {
  // Arrange
  const SegmentList intent_segments = {"intent-foo", "intent-bar"};
  const SegmentList interest_segments = {"interest-foo", "interest-bar"};

  // Act
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::EligibleAds eligible_ads(&subdivision_targeting,
                                             &anti_targeting_resource);

  eligible_ads.GetForFeatures(
      intent_segments, interest_segments,
      [=](const bool was_allowed,
          absl::optional<CreativeAdNotificationInfo> ad) {
        EXPECT_EQ(absl::nullopt, ad);
      });

  // Assert
}

TEST_F(BatAdsEligibleAdNotificationsTest, GetForFeaturesWithEmptySegments) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification_1 =
      GetCreativeAdNotificationForSegment("foo");
  creative_ad_notifications.push_back(creative_ad_notification_1);

  CreativeAdNotificationInfo creative_ad_notification_2 =
      GetCreativeAdNotificationForSegment("foo-bar");
  creative_ad_notifications.push_back(creative_ad_notification_2);

  Save(creative_ad_notifications);

  const SegmentList intent_segments = {};
  const SegmentList interest_segments = {};

  // Act
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::EligibleAds eligible_ads(&subdivision_targeting,
                                             &anti_targeting_resource);

  const CreativeAdNotificationInfo expected_ad = creative_ad_notification_2;

  eligible_ads.GetForFeatures(
      intent_segments, interest_segments,
      [=](const bool was_allowed,
          absl::optional<CreativeAdNotificationInfo> ad) { EXPECT_TRUE(ad); });

  // Assert
}

TEST_F(BatAdsEligibleAdNotificationsTest, GetForFeaturesDeterministically) {
  // Arrange
  CreativeAdNotificationList creative_ad_notifications;

  CreativeAdNotificationInfo creative_ad_notification_1 =
      GetCreativeAdNotificationForSegment("travel");
  creative_ad_notifications.push_back(creative_ad_notification_1);

  Log(creative_ad_notification_1, /* last seen in hours */ 48);

  CreativeAdNotificationInfo creative_ad_notification_2 =
      GetCreativeAdNotificationForSegment("travel-italy");
  creative_ad_notifications.push_back(creative_ad_notification_2);

  Log(creative_ad_notification_2, /* last seen in hours */ 16);

  Save(creative_ad_notifications);

  const SegmentList intent_segments = {"intent-foo", "intent-bar"};
  const SegmentList interest_segments = {"interest-foo", "interest-bar"};

  const char kAdFeatureWeights[] = "ad_feature_weights";
  std::map<std::string, std::string> kEligibleAdsParameters;
  // Bias distribution to deterministically show an ad
  kEligibleAdsParameters[kAdFeatureWeights] =
      "0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0";

  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitWithFeaturesAndParameters(
      {{features::kEligibleAds, kEligibleAdsParameters}}, {});

  // Act
  ad_targeting::geographic::SubdivisionTargeting subdivision_targeting;
  resource::AntiTargeting anti_targeting_resource;
  ad_notifications::EligibleAds eligible_ads(&subdivision_targeting,
                                             &anti_targeting_resource);

  const CreativeAdNotificationInfo expected_creative_ad_notification =
      creative_ad_notification_2;

  for (int i = 0; i < 10; i++) {
    eligible_ads.GetForFeatures(
        intent_segments, interest_segments,
        [=](const bool was_allowed,
            absl::optional<CreativeAdNotificationInfo> ad) {
          const CreativeAdNotificationInfo& creative_ad_notification =
              ad.value();
          EXPECT_EQ(expected_creative_ad_notification.creative_instance_id,
                    creative_ad_notification.creative_instance_id);
        });
  }
  // Assert
}

}  // namespace ads
