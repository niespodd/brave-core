/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/eligible_ads/ad_notifications/eligible_ad_notifications.h"

#include <map>
#include <vector>

#include "bat/ads/ad_notification_info.h"
#include "bat/ads/internal/ad_pacing/ad_pacing.h"
#include "bat/ads/internal/ad_priority/ad_priority.h"
#include "bat/ads/internal/ad_serving/ad_targeting/geographic/subdivision/subdivision_targeting.h"
#include "bat/ads/internal/ad_targeting/ad_targeting_segment_util.h"
#include "bat/ads/internal/ad_targeting/ad_targeting_values.h"
#include "bat/ads/internal/ads/ad_notifications/ad_notification_exclusion_rules.h"
#include "bat/ads/internal/ads_client_helper.h"
#include "bat/ads/internal/client/client.h"
#include "bat/ads/internal/database/tables/ad_events_database_table.h"
#include "bat/ads/internal/database/tables/creative_ad_notifications_database_table.h"
#include "bat/ads/internal/eligible_ads/ad_predictor_info.h"
#include "bat/ads/internal/eligible_ads/eligible_ads_predictor_util.h"
#include "bat/ads/internal/eligible_ads/eligible_ads_util.h"
#include "bat/ads/internal/eligible_ads/sample_ads.h"
#include "bat/ads/internal/eligible_ads/seen_ads.h"
#include "bat/ads/internal/eligible_ads/seen_advertisers.h"
#include "bat/ads/internal/features/ad_serving/ad_serving_features.h"
#include "bat/ads/internal/logging.h"
#include "bat/ads/internal/resources/frequency_capping/anti_targeting_resource.h"

namespace ads {
namespace ad_notifications {

namespace {

bool ShouldCapLastServedAd(const CreativeAdNotificationList& ads) {
  return ads.size() != 1;
}

}  // namespace

EligibleAds::EligibleAds(
    ad_targeting::geographic::SubdivisionTargeting* subdivision_targeting,
    resource::AntiTargeting* anti_targeting_resource)
    : subdivision_targeting_(subdivision_targeting),
      anti_targeting_resource_(anti_targeting_resource) {
  DCHECK(subdivision_targeting_);
  DCHECK(anti_targeting_resource_);
}

EligibleAds::~EligibleAds() = default;

void EligibleAds::SetLastServedAd(const CreativeAdInfo& creative_ad) {
  last_served_creative_ad_ = creative_ad;
}

void EligibleAds::GetForSegments(const SegmentList& segments,
                                 GetForSegmentsCallback callback) {
  database::table::AdEvents database_table;
  database_table.GetAll([=](const bool success, const AdEventList& ad_events) {
    if (!success) {
      BLOG(1, "Failed to get ad events");
      callback(/* was_allowed */ false, {});
      return;
    }

    const int max_count = features::GetBrowsingHistoryMaxCount();
    const int days_ago = features::GetBrowsingHistoryDaysAgo();
    AdsClientHelper::Get()->GetBrowsingHistory(
        max_count, days_ago, [=](const BrowsingHistoryList& history) {
          if (segments.empty()) {
            GetForUntargeted(ad_events, history, callback);
            return;
          }

          GetForParentChildSegments(segments, ad_events, history, callback);
        });
  });
}

void EligibleAds::GetForFeatures(const SegmentList& interest_segments,
                                 const SegmentList& intent_segments,
                                 GetForFeaturesCallback callback) {
  database::table::AdEvents database_table;
  database_table.GetAll([=](const bool success, const AdEventList& ad_events) {
    if (!success) {
      BLOG(1, "Failed to get ad events");
      callback(/* was_allowed */ false, absl::nullopt);
      return;
    }

    const int max_count = features::GetBrowsingHistoryMaxCount();
    const int days_ago = features::GetBrowsingHistoryDaysAgo();
    AdsClientHelper::Get()->GetBrowsingHistory(
        max_count, days_ago, [=](const BrowsingHistoryList& history) {
          GetEligibleAds(interest_segments, intent_segments, ad_events, history,
                         callback);
        });
  });
}

///////////////////////////////////////////////////////////////////////////////

void EligibleAds::GetEligibleAds(const SegmentList& interest_segments,
                                 const SegmentList& intent_segments,
                                 const AdEventList& ad_events,
                                 const BrowsingHistoryList& browsing_history,
                                 GetForFeaturesCallback callback) const {
  BLOG(1, "Get eligible ads");

  database::table::CreativeAdNotifications database_table;
  database_table.GetAll([=](const bool success, const SegmentList& segments,
                            const CreativeAdNotificationList& ads) {
    if (!success) {
      BLOG(1, "Failed to get ads");
      callback(/* was_allowed */ false, absl::nullopt);
      return;
    }

    if (ads.empty()) {
      BLOG(1, "No ads");
      callback(/* was_allowed */ true, absl::nullopt);
      return;
    }

    CreativeAdNotificationList eligible_ads = ApplyFrequencyCapping(
        ads,
        ShouldCapLastServedAd(ads) ? last_served_creative_ad_
                                   : CreativeAdInfo(),
        ad_events, browsing_history);

    if (eligible_ads.empty()) {
      BLOG(1, "No eligible ads");
      callback(/* was_allowed */ true, absl::nullopt);
      return;
    }

    ChooseAd(eligible_ads, ad_events, interest_segments, intent_segments,
             callback);
  });
}

void EligibleAds::ChooseAd(const CreativeAdNotificationList& eligible_ads,
                           const AdEventList& ad_events,
                           const SegmentList& interest_segments,
                           const SegmentList& intent_segments,
                           GetForFeaturesCallback callback) const {
  DCHECK(!eligible_ads.empty());

  std::map<std::string, AdPredictorInfo<CreativeAdNotificationInfo>> ads =
      GroupEligibleAdsByCreativeInstanceId(eligible_ads);

  std::map<std::string, AdPredictorInfo<CreativeAdNotificationInfo>>
      ads_with_features = ComputePredictorFeaturesAndScores(
          ads, ad_events, interest_segments, intent_segments);

  const absl::optional<CreativeAdNotificationInfo> ad =
      SampleFromAds(ads_with_features);

  callback(/* was_allowed */ true, ad);
}

void EligibleAds::GetForParentChildSegments(
    const SegmentList& segments,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history,
    GetForSegmentsCallback callback) const {
  DCHECK(!segments.empty());

  BLOG(1, "Get eligible ads for parent-child segments:");
  for (const auto& segment : segments) {
    BLOG(1, "  " << segment);
  }

  database::table::CreativeAdNotifications database_table;
  database_table.GetForSegments(
      segments, [=](const bool success, const SegmentList& segments,
                    const CreativeAdNotificationList& ads) {
        CreativeAdNotificationList eligible_ads =
            FilterIneligibleAds(ads, ad_events, browsing_history);

        if (eligible_ads.empty()) {
          BLOG(1, "No eligible ads for parent-child segments");
          GetForParentSegments(segments, ad_events, browsing_history, callback);
          return;
        }

        callback(/* was_allowed */ true, eligible_ads);
      });
}

void EligibleAds::GetForParentSegments(
    const SegmentList& segments,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history,
    GetForSegmentsCallback callback) const {
  DCHECK(!segments.empty());

  const SegmentList parent_segments = GetParentSegments(segments);
  if (parent_segments == segments) {
    callback(/* was_allowed */ false, {});
    return;
  }

  BLOG(1, "Get eligible ads for parent segments:");
  for (const auto& parent_segment : parent_segments) {
    BLOG(1, "  " << parent_segment);
  }

  database::table::CreativeAdNotifications database_table;
  database_table.GetForSegments(
      parent_segments, [=](const bool success, const SegmentList& segments,
                           const CreativeAdNotificationList& ads) {
        CreativeAdNotificationList eligible_ads =
            FilterIneligibleAds(ads, ad_events, browsing_history);

        if (eligible_ads.empty()) {
          BLOG(1, "No eligible ads for parent segments");
          GetForUntargeted(ad_events, browsing_history, callback);
          return;
        }

        callback(/* was_allowed */ true, eligible_ads);
      });
}

void EligibleAds::GetForUntargeted(const AdEventList& ad_events,
                                   const BrowsingHistoryList& browsing_history,
                                   GetForSegmentsCallback callback) const {
  BLOG(1, "Get eligble ads for untargeted segment");

  const std::vector<std::string> segments = {ad_targeting::kUntargeted};

  database::table::CreativeAdNotifications database_table;
  database_table.GetForSegments(
      segments, [=](const bool success, const SegmentList& segments,
                    const CreativeAdNotificationList& ads) {
        CreativeAdNotificationList eligible_ads =
            FilterIneligibleAds(ads, ad_events, browsing_history);

        if (eligible_ads.empty()) {
          BLOG(1, "No eligible ads for untargeted segment");
        }

        callback(/* was_allowed */ true, eligible_ads);
      });
}

CreativeAdNotificationList EligibleAds::FilterIneligibleAds(
    const CreativeAdNotificationList& ads,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history) const {
  if (ads.empty()) {
    return {};
  }

  CreativeAdNotificationList eligible_ads = ads;

  eligible_ads = FilterSeenAdvertisersAndRoundRobinIfNeeded(
      eligible_ads, AdType::kAdNotification);

  eligible_ads =
      FilterSeenAdsAndRoundRobinIfNeeded(eligible_ads, AdType::kAdNotification);

  eligible_ads = ApplyFrequencyCapping(
      eligible_ads,
      ShouldCapLastServedAd(ads) ? last_served_creative_ad_ : CreativeAdInfo(),
      ad_events, browsing_history);

  eligible_ads = PaceAds(eligible_ads);

  eligible_ads = PrioritizeAds(eligible_ads);

  return eligible_ads;
}

CreativeAdNotificationList EligibleAds::ApplyFrequencyCapping(
    const CreativeAdNotificationList& ads,
    const CreativeAdInfo& last_served_creative_ad,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history) const {
  CreativeAdNotificationList eligible_ads = ads;

  frequency_capping::ExclusionRules exclusion_rules(
      subdivision_targeting_, anti_targeting_resource_, ad_events,
      browsing_history);

  const auto iter = std::remove_if(
      eligible_ads.begin(), eligible_ads.end(),
      [&exclusion_rules, &last_served_creative_ad](CreativeAdInfo& ad) {
        return exclusion_rules.ShouldExcludeAd(ad) ||
               ad.creative_instance_id ==
                   last_served_creative_ad.creative_instance_id;
      });

  eligible_ads.erase(iter, eligible_ads.end());

  return eligible_ads;
}

}  // namespace ad_notifications
}  // namespace ads
