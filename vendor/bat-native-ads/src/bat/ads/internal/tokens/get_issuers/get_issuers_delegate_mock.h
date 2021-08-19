/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TOKENS_GET_ISSUERS_GET_ISSUERS_DELEGATE_MOCK_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TOKENS_GET_ISSUERS_GET_ISSUERS_DELEGATE_MOCK_H_

#include "bat/ads/internal/tokens/get_issuers/get_issuers_delegate.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace ads {

class GetIssuersDelegateMock : public GetIssuersDelegate {
 public:
  GetIssuersDelegateMock();

  ~GetIssuersDelegateMock() override;

  GetIssuersDelegateMock(const GetIssuersDelegateMock&) = delete;

  GetIssuersDelegateMock& operator=(const GetIssuersDelegateMock&) = delete;

  MOCK_METHOD(void, OnDidGetIssuers, ());

  MOCK_METHOD(void, OnFailedToGetIssuers, ());
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TOKENS_GET_ISSUERS_GET_ISSUERS_DELEGATE_MOCK_H_
