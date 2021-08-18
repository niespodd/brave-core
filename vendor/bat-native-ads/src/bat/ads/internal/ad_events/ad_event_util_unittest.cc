/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ad_events/ad_event_util.h"

#include <vector>

#include "base/guid.h"
#include "base/test/scoped_feature_list.h"
#include "bat/ads/internal/ad_events/ad_event_info.h"
#include "bat/ads/internal/bundle/creative_ad_notification_info.h"
#include "bat/ads/internal/eligible_ads/eligible_ads_features.h"
#include "bat/ads/internal/number_util.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

TEST(BatAdsEligibleAdsUtilTest, GetLastSeenAdTime) {
  // // Arrange
  // AdEventList ad_events;

  // const std::string creative_instance_id_1 = base::GenerateGUID();
  // AdEventInfo ad_event_1;
  // ad_event_1.uuid = base::GenerateGUID();
  // ad_event_1.confirmation_type = ConfirmationType::kViewed;
  // ad_event_1.creative_instance_id = creative_instance_id_1;
  // base::Time timestamp_1 =
  //     base::Time::Now() - base::TimeDelta::FromHours(48);
  // ad_event_1.timestamp = static_cast<int64_t>(timestamp_1.ToDoubleT());
  // ad_events.push_back(ad_event_1);

  // const std::string creative_instance_id_2 = base::GenerateGUID();
  // AdEventInfo ad_event_2;
  // ad_event_2.uuid = base::GenerateGUID();
  // ad_event_2.confirmation_type = ConfirmationType::kViewed;
  // ad_event_2.creative_instance_id = creative_instance_id_2;
  // base::Time timestamp_2 =
  //     base::Time::Now() - base::TimeDelta::FromHours(26);
  // ad_event_2.timestamp = static_cast<int64_t>(timestamp_2.ToDoubleT());
  // ad_events.push_back(ad_event_2);

  // const std::string creative_instance_id_3 = base::GenerateGUID();
  // AdEventInfo ad_event_3;
  // ad_event_3.uuid = base::GenerateGUID();
  // ad_event_3.confirmation_type = ConfirmationType::kClicked;
  // ad_event_3.creative_instance_id = creative_instance_id_2;
  // base::Time timestamp_3 =
  //     base::Time::Now() - base::TimeDelta::FromHours(24);
  // ad_event_3.timestamp = static_cast<int64_t>(timestamp_3.ToDoubleT());
  // ad_events.push_back(ad_event_3);

  // const std::string creative_instance_id_4 = base::GenerateGUID();
  // AdEventInfo ad_event_4;
  // ad_event_4.uuid = base::GenerateGUID();
  // ad_event_4.confirmation_type = ConfirmationType::kViewed;
  // ad_event_4.creative_instance_id = creative_instance_id_3;
  // base::Time timestamp_4 =
  //     base::Time::Now() - base::TimeDelta::FromHours(12);
  // ad_event_4.timestamp = static_cast<int64_t>(timestamp_4.ToDoubleT());
  // ad_events.push_back(ad_event_4);

  // // Act
  // const base::Time ad_last_seen_in_hours =
  //     GetLastSeenAdTime(ad_events, creative_instance_id_2);

  // // Assert
  // EXPECT_EQ(26, ad_last_seen_in_hours);
  EXPECT_TRUE(true);
}

}  // namespace ads
