/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/tokens/get_issuers/get_issuers.h"
#include "bat/ads/internal/tokens/get_issuers/get_issuers_delegate_mock.h"
#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"
#include "net/http/http_status_code.h"

// npm run test -- brave_unit_tests --filter=BatAds*

using ::testing::_;
using ::testing::NiceMock;

namespace ads {

class BatAdsGetIssuersTest : public UnitTestBase {
 protected:
  BatAdsGetIssuersTest()
      : get_issuers_(std::make_unique<GetIssuers>()),
        get_issuers_delegate_mock_(
            std::make_unique<NiceMock<GetIssuersDelegateMock>>()) {
    get_issuers_->set_delegate(get_issuers_delegate_mock_.get());
  }

  ~BatAdsGetIssuersTest() override = default;

  std::unique_ptr<GetIssuers> get_issuers_;
  std::unique_ptr<GetIssuersDelegateMock> get_issuers_delegate_mock_;
};

TEST_F(BatAdsGetIssuersTest, SucceededToGetIssuers) {
  // Arrange
  const URLEndpoints endpoints = {{// Get issuers request
                                   R"(/v1/issuers/)",
                                   {{net::HTTP_OK, R"(
        [
          {
            "name": "confirmations",
            "publicKeys": [
              "JsvJluEN35bJBgJWTdW/8dAgPrrTM1I1pXga+o7cllo="
            ]
          },
          {
            "name": "payments",
            "publicKeys": [
              "JiwFR2EU/Adf1lgox+xqOVPuc6a/rxdy/LguFG5eaXg=",
              "XgxwreIbLMu0IIFVk4TKEce6RduNVXngDmU3uixly0M=",
              "CrQLMWmUuYog6Q93nScS8Lo1HHSex8WM2Qxij7qhjkQ=",
              "FnEJA85KqpxGaOkHiIzldRIUfaBe5etyUn2ThCpZKS0=",
              "lLO5tErGoTK0askrALab6pKGAnBHqELSyw/evqZRwH8=",
              "6DBiZUS47m8eb5ohI2MiRaERLzS4DQgMp4nxPLKAenA=",
              "YOIEGq4joK7rtkWdcNdNNGT5xlU/KIrri4AX19qwZW4=",
              "VihGXGoiQ5Fjxe4SrskIVMcmERa1LoAgvhFxxfLmNEI=",
              "iJcG3AkH1sgl+5YCZuo+4Q/7aeBOnYyntkIUXeMbDCs=",
              "aDD4SJmIj2xwdA6D00K1dopTg90oOFpwd2iiK8bqqlQ=",
              "bPE1QE65mkIgytffeu7STOfly+x10BXCGuk5pVlOHQU="
            ]
          }
        ]
        )"}}}};

  MockUrlRequest(ads_client_mock_, endpoints);

  // Act
  EXPECT_CALL(*get_issuers_delegate_mock_, OnDidGetIssuers()).Times(1);

  EXPECT_CALL(*get_issuers_delegate_mock_, OnFailedToGetIssuers()).Times(0);

  get_issuers_->RequestIssuers(nullptr);

  // Assert
  EXPECT_TRUE(get_issuers_->IsInitialized());
}

}  // namespace ads
