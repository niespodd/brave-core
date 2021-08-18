// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/brave_ads/brave_ads_host_private.h"

#include <utility>

namespace brave_ads {

BraveAdsHostPrivate::BraveAdsHostPrivate() = default;

BraveAdsHostPrivate::~BraveAdsHostPrivate() = default;

void BraveAdsHostPrivate::RequestAdsEnabled(
    RequestAdsEnabledCallback callback) {
  std::move(callback).Run(false);
}

}  // namespace brave_ads
