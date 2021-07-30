// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_BRAVE_ADS_BRAVE_ADS_DRIVER_PRIVATE_H_
#define BRAVE_BROWSER_BRAVE_ADS_BRAVE_ADS_DRIVER_PRIVATE_H_

#include "brave/components/brave_ads/common/brave_ads_driver.mojom.h"

namespace brave_ads {

class BraveAdsDriverPrivate final : public brave_ads::mojom::BraveAdsDriver {
 public:
  BraveAdsDriverPrivate();
  BraveAdsDriverPrivate(const BraveAdsDriverPrivate&) = delete;
  BraveAdsDriverPrivate& operator=(const BraveAdsDriverPrivate&) = delete;
  ~BraveAdsDriverPrivate() override;

  // brave_ads::mojom::BraveAdsDriver
  void RequestAdsEnabled(RequestAdsEnabledCallback callback) override;
};

}  // namespace brave_ads

#endif  // BRAVE_BROWSER_BRAVE_ADS_BRAVE_ADS_DRIVER_PRIVATE_H_
