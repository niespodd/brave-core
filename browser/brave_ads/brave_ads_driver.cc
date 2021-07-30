/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_ads/brave_ads_driver.h"

#include <memory>
#include <string>
#include <utility>

#include "brave/browser/brave_ads/ads_service_factory.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/browser/extensions/api/brave_action_api.h"
#include "brave/browser/extensions/brave_component_loader.h"
#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/brave_rewards/browser/rewards_service.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/constants.h"

namespace brave_ads {

namespace {

constexpr char kAdsEnableRelativeUrl[] =
    "request_ads_enabled_panel.html#enable_ads";

}  // namespace

BraveAdsDriver::BraveAdsDriver(Profile* profile,
                               content::WebContents* web_contents)
    : WebContentsObserver(web_contents),
      profile_(profile),
      web_contents_(web_contents) {
  DCHECK(profile_);
  DCHECK(web_contents_);

  browser_ = chrome::FindBrowserWithWebContents(web_contents_);
  DCHECK(browser_);
}

BraveAdsDriver::~BraveAdsDriver() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(!callback_);
}

void BraveAdsDriver::RequestAdsEnabled(RequestAdsEnabledCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (callback_) {
    std::move(callback).Run(false);
    return;
  }

  AdsService* ads_service_ = AdsServiceFactory::GetForProfile(profile_);
  brave_rewards::RewardsService* rewards_service_ =
      brave_rewards::RewardsServiceFactory::GetForProfile(profile_);

  if (!ads_service_ || !rewards_service_) {
    std::move(callback_).Run(false);
    return;
  }

  callback_ = std::move(callback);

  const bool is_rewards_enabled = rewards_service_->IsRewardsEnabled();
  const bool is_ads_enabled = ads_service_->IsEnabled();
  if (is_rewards_enabled && is_ads_enabled) {
    std::move(callback_).Run(true);
    return;
  }

  rewards_service_->StartProcess(base::DoNothing());
  rewards_service_observation_.Observe(rewards_service_);

  if (!ShowRewardsPopup()) {
    Reset();
    std::move(callback_).Run(false);
  }
}

void BraveAdsDriver::WebContentsDestroyed() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  Reset();

  if (callback_) {
    std::move(callback_).Run(false);
  }
}

void BraveAdsDriver::OnProcessForEnableRewardsStarted() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!callback_) {
    NOTREACHED();
    return;
  }

  process_for_enable_rewards_started_ = true;
}

void BraveAdsDriver::OnRewardsPanelClosed(Browser* browser) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (browser_ != browser) {
    return;
  }

  if (process_for_enable_rewards_started_ || !callback_) {
    return;
  }

  Reset();

  std::move(callback_).Run(false);
}

void BraveAdsDriver::OnAdsEnabled(
    brave_rewards::RewardsService* rewards_service,
    bool ads_enabled) {
  DCHECK(rewards_service);
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  Reset();

  if (!callback_) {
    NOTREACHED();
    return;
  }

  std::move(callback_).Run(ads_enabled);
}

bool BraveAdsDriver::ShowRewardsPopup() {
  auto* extension_service =
      extensions::ExtensionSystem::Get(profile_)->extension_service();
  if (!extension_service) {
    return false;
  }

  static_cast<extensions::BraveComponentLoader*>(
      extension_service->component_loader())
      ->AddRewardsExtension();

  std::string error;
  return extensions::BraveActionAPI::ShowActionUI(
      browser_, brave_rewards_extension_id,
      std::make_unique<std::string>(kAdsEnableRelativeUrl), &error);
}

void BraveAdsDriver::Reset() {
  rewards_service_observation_.Reset();
  process_for_enable_rewards_started_ = false;
}

}  // namespace brave_ads
