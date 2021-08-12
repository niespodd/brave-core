/* Copyright 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "services/network/network_service_network_delegate.h"

#define OnCanGetCookies OnCanGetCookies_ChromiumImpl
#define OnCanSetCookie OnCanSetCookie_ChromiumImpl
#define OnForcePrivacyMode OnForcePrivacyMode_ChromiumImpl

#include "../../../../services/network/network_service_network_delegate.cc"

#undef OnForcePrivacyMode
#undef OnCanSetCookie
#undef OnCanGetCookies

using content_settings::EphemeralStorageAwareType;

namespace network {

bool NetworkServiceNetworkDelegate::OnCanGetCookies(
    const net::URLRequest& request,
    bool allowed_from_caller) {
  // Enable ephemeral storage support for the call.
  auto scoped_ephemeral_storage_awareness =
      network_context_->cookie_manager()
          ->cookie_settings()
          .CreateScopedEphemeralStorageAwareness(
              EphemeralStorageAwareType::kAware);
  return OnCanGetCookies_ChromiumImpl(request, allowed_from_caller);
}

bool NetworkServiceNetworkDelegate::OnCanSetCookie(
    const net::URLRequest& request,
    const net::CanonicalCookie& cookie,
    net::CookieOptions* options,
    bool allowed_from_caller) {
  // Enable ephemeral storage support for the call.
  auto scoped_ephemeral_storage_awareness =
      network_context_->cookie_manager()
          ->cookie_settings()
          .CreateScopedEphemeralStorageAwareness(
              EphemeralStorageAwareType::kAware);
  return OnCanSetCookie_ChromiumImpl(request, cookie, options,
                                     allowed_from_caller);
}

bool NetworkServiceNetworkDelegate::OnForcePrivacyMode(
    const GURL& url,
    const net::SiteForCookies& site_for_cookies,
    const absl::optional<url::Origin>& top_frame_origin) const {
  // Enable ephemeral storage support for the call.
  auto scoped_ephemeral_storage_awareness =
      network_context_->cookie_manager()
          ->cookie_settings()
          .CreateScopedEphemeralStorageAwareness(
              EphemeralStorageAwareType::kNotAwareButAllowIn1pEphemeralMode);
  return OnForcePrivacyMode_ChromiumImpl(url, site_for_cookies,
                                         top_frame_origin);
}

}  // namespace network
