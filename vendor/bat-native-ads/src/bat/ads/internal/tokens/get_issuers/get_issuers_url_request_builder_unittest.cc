/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <set>
#include <vector>

#include "base/json/json_reader.h"
#include "base/values.h"
#include "bat/ads/internal/tokens/get_issuers/get_issuers_url_request_builder.h"
#include "bat/ads/internal/unittest_base.h"

#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

using challenge_bypass_ristretto::PublicKey;
using challenge_bypass_ristretto::UnblindedToken;

class BatAdsGetIssuersUrlRequestBuilderTest : public UnitTestBase {
 protected:
  BatAdsGetIssuersUrlRequestBuilderTest() = default;

  ~BatAdsGetIssuersUrlRequestBuilderTest() override = default;
};

TEST(BatAdsGetIssuersUrlRequestBuilderTest, BuildUrl) {
  // Arrange
  GetIssuersUrlRequestBuilder url_request_builder;

  // Act
  UrlRequestPtr url_request = url_request_builder.Build();

  // Assert
  UrlRequestPtr expected_url_request = UrlRequest::New();
  expected_url_request->url =
      R"(https://ads-serve.brave.software)/v1/issuers/)";
  expected_url_request->method = UrlRequestMethod::GET;

  EXPECT_EQ(expected_url_request, url_request);
}

}  // namespace ads
