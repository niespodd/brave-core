/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_EVENTS_AD_EVENT_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_EVENTS_AD_EVENT_UTIL_H_

#include "bat/ads/internal/ad_events/ad_event_info.h"

#include <algorithm>

#include "base/time/time.h"

namespace ads {

struct AdInfo;

bool HasFiredAdViewedEvent(const AdInfo& ad, const AdEventList& ad_events);

template <typename T>
base::Time GetLastSeenAdTime(const AdEventList& ad_events,
                             const T& creative_ad) {
  const auto iter = std::find_if(
      ad_events.begin(), ad_events.end(),
      [&creative_ad](const AdEventInfo& ad_event) -> bool {
        return (ad_event.creative_instance_id ==
                    creative_ad.creative_instance_id &&
                ad_event.confirmation_type == ConfirmationType::kViewed);
      });

  double timestamp = 0;
  if (iter != ad_events.end()) {
    timestamp = iter->timestamp;
  }

  return base::Time::FromDoubleT(timestamp);
}

template <typename T>
base::Time GetLastSeenAdvertiserTime(const AdEventList& ad_events,
                                     const T& creative_ad) {
  const auto iter = std::find_if(
      ad_events.begin(), ad_events.end(),
      [&creative_ad](const AdEventInfo& ad_event) -> bool {
        return (ad_event.advertiser_id == creative_ad.advertiser_id &&
                ad_event.confirmation_type == ConfirmationType::kViewed);
      });

  double timestamp = 0;
  if (iter != ad_events.end()) {
    timestamp = iter->timestamp;
  }

  return base::Time::FromDoubleT(timestamp);
}

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_EVENTS_AD_EVENT_UTIL_H_
