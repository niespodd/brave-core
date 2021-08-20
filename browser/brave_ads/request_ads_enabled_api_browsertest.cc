/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/path_service.h"
#include "base/test/scoped_feature_list.h"
#include "brave/browser/brave_ads/ads_service_factory.h"
#include "brave/common/brave_paths.h"
#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/brave_ads/common/features.h"
#include "brave/components/brave_rewards/browser/test/common/rewards_browsertest_context_util.h"
#include "brave/components/brave_rewards/browser/test/common/rewards_browsertest_util.h"
#include "brave/components/brave_rewards/common/pref_names.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "extensions/common/constants.h"
#include "net/dns/mock_host_resolver.h"

// npm run test -- brave_browser_tests --filter=RequestAdsEnabledApiTest*

namespace {

constexpr char kAllowedDomain[] = "talk.brave.com";
constexpr char kNotAllowedDomain[] = "brave.com";
constexpr char kBraveRequestAdsEnabledExists[] =
    "window.domAutomationController.send("
    "  !!(window.chrome && window.chrome.braveRequestAdsEnabled)"
    ")";
constexpr char kBraveRequestAdsEnabled[] =
    "let request_promise = window.chrome.braveRequestAdsEnabled()";
constexpr char kResolveRequestAdsEnabledPromise[] =
    "request_promise.then(function (enabled) {"
    "  window.domAutomationController.send(enabled)"
    "})";
constexpr char kGetRequestAdsEnabledPromiseRejectReason[] =
    "window.chrome.braveRequestAdsEnabled().then("
    "    undefined,"
    "    function (reason) {"
    "        window.domAutomationController.send(reason)"
    "    })";
constexpr char kUserGestureRejectReason[] =
    "braveRequestAdsEnabled: API can only be initiated by a user gesture.";

}  // namespace

class RequestAdsEnabledApiTestBase : public InProcessBrowserTest {
 public:
  RequestAdsEnabledApiTestBase() {}

  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();
    host_resolver()->AddRule("*", "127.0.0.1");

    https_server_.reset(new net::EmbeddedTestServer(
        net::test_server::EmbeddedTestServer::TYPE_HTTPS));
    https_server_->SetSSLConfig(net::EmbeddedTestServer::CERT_OK);

    brave::RegisterPathProvider();
    base::FilePath test_data_dir;
    base::PathService::Get(brave::DIR_TEST_DATA, &test_data_dir);
    https_server_->ServeFilesFromDirectory(test_data_dir);

    ASSERT_TRUE(https_server_->Start());
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    // HTTPS server only serves a valid cert for localhost, so this is needed
    // to load pages from other hosts without an error.
    command_line->AppendSwitch(switches::kIgnoreCertificateErrors);
  }

  content::WebContents* OpenRequestAdsEnabledPopup(
      content::WebContents* contents,
      const std::string& enable_button_test_id) {
    content::WebContents* popup_contents = nullptr;
    auto check_load_is_rewards_panel =
        [](content::WebContents** popup_contents,
           const content::NotificationSource& source,
           const content::NotificationDetails&) -> bool {
      auto web_contents_source =
          static_cast<const content::Source<content::WebContents>&>(source);
      *popup_contents = web_contents_source.ptr();

      // Check that this notification is for the Rewards panel and not, say,
      // the extension background page.
      std::string url = (*popup_contents)->GetLastCommittedURL().spec();
      std::string rewards_panel_url = std::string("chrome-extension://") +
                                      brave_rewards_extension_id +
                                      "/request_ads_enabled_panel.html";
      return url == rewards_panel_url;
    };

    content::WindowedNotificationObserver popup_observer(
        content::NOTIFICATION_LOAD_COMPLETED_MAIN_FRAME,
        base::BindRepeating(check_load_is_rewards_panel, &popup_contents));

    EXPECT_TRUE(ExecuteScript(contents, kBraveRequestAdsEnabled));

    // Wait for the popup to load
    popup_observer.Wait();
    rewards_browsertest_util::WaitForElementToAppear(
        popup_contents, std::string("[data-test-id=")
                            .append(enable_button_test_id)
                            .append("]"));

    return popup_contents;
  }

  net::EmbeddedTestServer* https_server() { return https_server_.get(); }

 protected:
  base::test::ScopedFeatureList feature_list_;

 private:
  std::unique_ptr<net::EmbeddedTestServer> https_server_;
};

class RequestAdsEnabledApiTestEnabled : public RequestAdsEnabledApiTestBase {
 public:
  RequestAdsEnabledApiTestEnabled() {
    feature_list_.InitAndEnableFeature(
        brave_ads::features::kRequestAdsEnabledApi);
  }
};

IN_PROC_BROWSER_TEST_F(RequestAdsEnabledApiTestEnabled, AdsAlreadyEnabled) {
  brave_ads::AdsService* ads_service =
      brave_ads::AdsServiceFactory::GetForProfile(browser()->profile());
  ads_service->SetEnabled(true);

  GURL url = https_server()->GetURL(kAllowedDomain, "/simple.html");

  ui_test_utils::NavigateToURL(browser(), url);
  content::WebContents* contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  WaitForLoadStop(contents);
  EXPECT_EQ(url, contents->GetURL());
  bool has_api;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kBraveRequestAdsEnabledExists, &has_api));
  EXPECT_TRUE(has_api);

  EXPECT_TRUE(ExecuteScript(contents, kBraveRequestAdsEnabled));

  bool enabled;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kResolveRequestAdsEnabledPromise, &enabled));
  EXPECT_TRUE(enabled);
}

IN_PROC_BROWSER_TEST_F(RequestAdsEnabledApiTestEnabled, PopupAccepted) {
  GURL url = https_server()->GetURL(kAllowedDomain, "/simple.html");

  ui_test_utils::NavigateToURL(browser(), url);
  content::WebContents* contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  WaitForLoadStop(contents);
  EXPECT_EQ(url, contents->GetURL());
  bool has_api;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kBraveRequestAdsEnabledExists, &has_api));
  EXPECT_TRUE(has_api);

  content::WebContents* popup_contents =
      OpenRequestAdsEnabledPopup(contents, "brave-talk-turn-on-rewards");

  rewards_browsertest_util::WaitForElementThenClick(
      popup_contents, "[data-test-id=brave-talk-turn-on-rewards]");

  bool enabled;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kResolveRequestAdsEnabledPromise, &enabled));
  EXPECT_TRUE(enabled);
}

IN_PROC_BROWSER_TEST_F(RequestAdsEnabledApiTestEnabled,
                       PopupClosedByNewTabOpen) {
  browser()->profile()->GetPrefs()->SetBoolean(
      brave_rewards::prefs::kAutoContributeEnabled, true);

  GURL url = https_server()->GetURL(kAllowedDomain, "/simple.html");

  ui_test_utils::NavigateToURL(browser(), url);
  content::WebContents* contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  WaitForLoadStop(contents);
  EXPECT_EQ(url, contents->GetURL());
  bool has_api;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kBraveRequestAdsEnabledExists, &has_api));
  EXPECT_TRUE(has_api);

  OpenRequestAdsEnabledPopup(contents, "brave-talk-turn-on-private-ads");

  ui_test_utils::NavigateToURLWithDisposition(
      browser(), GURL("about:blank"), WindowOpenDisposition::NEW_FOREGROUND_TAB,
      ui_test_utils::BROWSER_TEST_WAIT_FOR_LOAD_STOP);

  bool enabled;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kResolveRequestAdsEnabledPromise, &enabled));
  EXPECT_FALSE(enabled);
}

IN_PROC_BROWSER_TEST_F(RequestAdsEnabledApiTestEnabled,
                       ApiForIncognitoBrowser) {
  GURL url = https_server()->GetURL(kAllowedDomain, "/simple.html");

  Browser* incognito_browser = OpenURLOffTheRecord(browser()->profile(), url);

  content::WebContents* contents =
      incognito_browser->tab_strip_model()->GetActiveWebContents();
  WaitForLoadStop(contents);
  EXPECT_EQ(url, contents->GetURL());

  bool has_api;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kBraveRequestAdsEnabledExists, &has_api));
  EXPECT_TRUE(has_api);

  EXPECT_TRUE(ExecuteScript(contents, kBraveRequestAdsEnabled));

  bool enabled;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kResolveRequestAdsEnabledPromise, &enabled));
  EXPECT_FALSE(enabled);
}

IN_PROC_BROWSER_TEST_F(RequestAdsEnabledApiTestEnabled,
                       RunApiForWithoutUserGesture) {
  GURL url = https_server()->GetURL(kAllowedDomain, "/simple.html");

  ui_test_utils::NavigateToURL(browser(), url);
  content::WebContents* contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  WaitForLoadStop(contents);
  EXPECT_EQ(url, contents->GetURL());

  bool has_api;
  EXPECT_TRUE(ExecuteScriptWithoutUserGestureAndExtractBool(
      contents, kBraveRequestAdsEnabledExists, &has_api));
  EXPECT_TRUE(has_api);

  std::string status;
  EXPECT_TRUE(ExecuteScriptWithoutUserGestureAndExtractString(
      contents, kGetRequestAdsEnabledPromiseRejectReason, &status));
  EXPECT_EQ(status, kUserGestureRejectReason);
}

IN_PROC_BROWSER_TEST_F(RequestAdsEnabledApiTestEnabled,
                       ApiNotAvailableForUnknownHost) {
  GURL url = https_server()->GetURL(kNotAllowedDomain, "/simple.html");

  ui_test_utils::NavigateToURL(browser(), url);
  content::WebContents* contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  WaitForLoadStop(contents);
  EXPECT_EQ(url, contents->GetURL());

  bool has_api;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kBraveRequestAdsEnabledExists, &has_api));
  EXPECT_FALSE(has_api);
}

class RequestAdsEnabledApiTestDisabled : public RequestAdsEnabledApiTestBase {
 public:
  RequestAdsEnabledApiTestDisabled() {
    feature_list_.InitAndDisableFeature(
        brave_ads::features::kRequestAdsEnabledApi);
  }
};

IN_PROC_BROWSER_TEST_F(RequestAdsEnabledApiTestDisabled,
                       ApiNotAvailableWhenFeatureOff) {
  GURL url = https_server()->GetURL(kAllowedDomain, "/simple.html");

  ui_test_utils::NavigateToURL(browser(), url);
  content::WebContents* contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  WaitForLoadStop(contents);
  EXPECT_EQ(url, contents->GetURL());

  bool has_api;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(
      contents, kBraveRequestAdsEnabledExists, &has_api));
  EXPECT_FALSE(has_api);
}
