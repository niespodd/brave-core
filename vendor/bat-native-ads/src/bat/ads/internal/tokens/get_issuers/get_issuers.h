/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TOKENS_GET_ISSUERS_GET_ISSUERS_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TOKENS_GET_ISSUERS_GET_ISSUERS_H_

#include <functional>
#include <map>
#include <set>
#include <string>

#include "base/callback.h"
#include "bat/ads/internal/account/confirmations/confirmation_info.h"
#include "bat/ads/internal/logging.h"
#include "brave/components/services/bat_ads/public/interfaces/bat_ads.mojom.h"
#include "bat/ads/internal/tokens/get_issuers/get_issuers_delegate.h"

namespace ads {

using ConfirmationCallback = std::function<void()>;

class GetIssuers {
 public:
  GetIssuers();

  ~GetIssuers();

  void set_delegate(GetIssuersDelegate* delegate);

  void RequestIssuers(const ConfirmationCallback& callback);

  bool PublicKeyExists(const std::string& issuer_name,
                       const std::string& public_key);

  bool IsInitialized();

 private:
  void OnRequestIssuers(const mojom::UrlResponse& url_response,
                        const ConfirmationCallback& callback);

  bool ParseResponseBody(const mojom::UrlResponse& url_response);

  void OnDidGetIssuers();

  void OnFailedToGetIssuers();

  std::map<std::string, std::set<std::string>> issuer_name_to_public_keys_;
  bool initialized_;

  GetIssuersDelegate* delegate_ = nullptr;
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TOKENS_GET_ISSUERS_GET_ISSUERS_H_
