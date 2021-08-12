/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_COMPONENTS_CONTENT_SETTINGS_CORE_COMMON_COOKIE_SETTINGS_BASE_H_
#define BRAVE_CHROMIUM_SRC_COMPONENTS_CONTENT_SETTINGS_CORE_COMMON_COOKIE_SETTINGS_BASE_H_

#include "base/auto_reset.h"

namespace content_settings {

enum class EphemeralStorageAwareType {
  kNone,
  kAware,
  kNotAwareButAllowIn1pEphemeralMode,
};

// Helper to allow patchless ephemeral storage access in Chromium code.
using ScopedEphemeralStorageAwareness =
    base::AutoReset<EphemeralStorageAwareType>;

}  // namespace content_settings

#define BRAVE_COOKIE_SETTINGS_BASE_H                                      \
  bool ShouldUseEphemeralStorage(                                         \
      const GURL& url, const GURL& site_for_cookies,                      \
      const absl::optional<url::Origin>& top_frame_origin) const;         \
  ScopedEphemeralStorageAwareness CreateScopedEphemeralStorageAwareness(  \
      EphemeralStorageAwareType type) const;                              \
  bool IsEphemeralCookieAccessAllowed(const GURL& url,                    \
                                      const GURL& first_party_url) const; \
  bool IsEphemeralCookieAccessAllowed(                                    \
      const GURL& url, const GURL& site_for_cookies,                      \
      const absl::optional<url::Origin>& top_frame_origin) const;         \
  bool IsChromiumCookieAccessAllowed(const GURL& url,                     \
                                     const GURL& first_party_url) const;  \
  bool IsChromiumCookieAccessAllowed(                                     \
      const GURL& url, const GURL& site_for_cookies,                      \
      const absl::optional<url::Origin>& top_frame_origin) const;         \
                                                                          \
 private:                                                                 \
  bool IsCookieAccessAllowedImpl(                                         \
      const GURL& url, const GURL& site_for_cookies,                      \
      const absl::optional<url::Origin>& top_frame_origin) const;         \
                                                                          \
  mutable EphemeralStorageAwareType ephemeral_storage_aware_ =            \
      EphemeralStorageAwareType::kNone;

#include "../../../../../../components/content_settings/core/common/cookie_settings_base.h"

#undef BRAVE_COOKIE_SETTINGS_BASE_H

#endif  // BRAVE_CHROMIUM_SRC_COMPONENTS_CONTENT_SETTINGS_CORE_COMMON_COOKIE_SETTINGS_BASE_H_
