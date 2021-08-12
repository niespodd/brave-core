/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Host, GrantInfo, GrantCaptchaStatus } from './interfaces'
import { OpenLinkAction, ClaimGrantAction } from '../../shared/components/notifications'
import { ExternalWalletAction } from '../../shared/components/wallet_card'
import * as apiAdapter from './extension_api_adapter'
import { getInitialState } from './initial_state'
import { createStateManager } from '../../shared/lib/state_manager'

type LocaleStorageKey =
  'hide-publisher-unverified-note' |
  'notifications-last-viewed'

function readLocalStorage (key: LocaleStorageKey): unknown {
  try {
    return JSON.parse(localStorage.getItem(`rewards-panel-${key}`) || '')
  } catch {
    return null
  }
}

function writeLocalStorage (key: LocaleStorageKey, value: unknown) {
  localStorage.setItem(`rewards-panel-${key}`, JSON.stringify(value))
}

function closePanel () {
  window.close()
}

function getCurrentTabInfo () {
  interface Result {
    id: number
  }

  return new Promise<Result | null>((resolve) => {
    chrome.tabs.query({ active: true, currentWindow: true }, (tabs) => {
      if (tabs.length > 0) {
        const { id } = tabs[0]
        if (typeof id === 'number') {
          resolve({ id })
          return
        }
      }
      resolve(null)
    })
  })
}

function openTab (url: string) {
  if (!url) {
    console.error(new Error('Cannot open a tab with an empty URL'))
    return
  }
  chrome.tabs.create({ url }, () => { closePanel() })
}

export function createHost (): Host {
  const stateManager = createStateManager(getInitialState())
  const grants = new Map<string, GrantInfo>()

  async function updatePublisherInfo () {
    const tabInfo = await getCurrentTabInfo()
    if (tabInfo) {
      stateManager.update({
        publisherInfo: await apiAdapter.getPublisherInfo(tabInfo.id)
      })
    }
  }

  function loadCaptcha (grantId: string, status: GrantCaptchaStatus) {
    const grantInfo = grants.get(grantId)
    if (!grantInfo) {
      stateManager.update({ grantCaptchaInfo: null })
      return
    }

    stateManager.update({
      grantCaptchaInfo: {
        id: '',
        hint: '',
        imageURL: '',
        status,
        grantInfo
      }
    })

    chrome.braveRewards.claimPromotion(grantId, (properties) => {
      stateManager.update({
        grantCaptchaInfo: {
          id: properties.captchaId,
          hint: properties.hint,
          imageURL: properties.captchaImage,
          status,
          grantInfo
        }
      })
    })
  }

  function handleExternalWalletAction (action: ExternalWalletAction) {
    const { externalWallet } = stateManager.getState()

    if (action === 'disconnect') {
      if (externalWallet) {
        chrome.braveRewards.disconnectWallet()
        stateManager.update({ externalWallet: null })
      }
      return
    }

    Promise.resolve().then(() => {
      const verifyURL = 'chrome://rewards#verify'
      if (!externalWallet) {
        return verifyURL
      }

      const { links } = externalWallet
      switch (action) {
        case 'add-funds':
          return links.addFunds || links.account
        case 'complete-verification':
          return links.completeVerification || links.account
        case 'reconnect':
          return apiAdapter.getExternalWalletLoginURL(externalWallet.provider)
        case 'verify':
          return verifyURL
        case 'view-account':
          return links.account
      }
    }).then(openTab, console.error)
  }

  function processURL () {
    const { hash } = window.location
    const grantMatch = hash.match(/^#?grant_([\s\S]+)/)
    if (grantMatch) {
      const grantId = grantMatch[1]
      loadCaptcha(grantId, 'pending')
    }
  }

  async function initialize () {
    apiAdapter.onGrantsUpdated((list) => {
      grants.clear()
      for (const grant of list) {
        grants.set(grant.id, grant)
      }
    })

    chrome.braveRewards.onUnblindedTokensReady.addListener(() => {
      apiAdapter.getRewardsBalance().then((balance) => {
        stateManager.update({ balance })
      }).catch(console.error)
    })

    function updateNotifications () {
      apiAdapter.getNotifications().then((notifications) => {
        stateManager.update({ notifications })
      }).catch(console.error)
    }

    chrome.rewardsNotifications.onAllNotificationsDeleted.addListener(
      updateNotifications)
    chrome.rewardsNotifications.onNotificationAdded.addListener(
      updateNotifications)
    chrome.rewardsNotifications.onNotificationDeleted.addListener(
      updateNotifications)

    stateManager.update({
      hidePublisherUnverifiedNote:
        Boolean(readLocalStorage('hide-publisher-unverified-note')),
      notificationsLastViewed:
        Number(readLocalStorage('notifications-last-viewed')) || 0
    })

    await Promise.all([
      apiAdapter.getRewardsEnabled().then((rewardsEnabled) => {
        stateManager.update({ rewardsEnabled })
      }),
      apiAdapter.getRewardsBalance().then((balance) => {
        stateManager.update({ balance })
      }),
      apiAdapter.getRewardsParameters().then((params) => {
        const { options, exchangeInfo } = params
        stateManager.update({ options, exchangeInfo })
      }),
      apiAdapter.getSettings().then((settings) => {
        stateManager.update({ settings })
      }),
      apiAdapter.getExternalWalletProviders().then((providers) => {
        stateManager.update({ externalWalletProviders: providers })
      }),
      apiAdapter.getExternalWallet().then((externalWallet) => {
        stateManager.update({ externalWallet })
      }),
      apiAdapter.getEarningsInfo().then((earningsInfo) => {
        if (earningsInfo) {
          stateManager.update({ earningsInfo })
        }
      }),
      apiAdapter.getRewardsSummaryData().then((summaryData) => {
        stateManager.update({ summaryData })
      }),
      apiAdapter.getNotifications().then((notifications) => {
        stateManager.update({ notifications })
      }),
      updatePublisherInfo()
    ])

    stateManager.update({ loading: false })

    processURL()
  }

  initialize().catch((error) => {
    console.error(error)
    // TODO(zenparsing): Error UX
  })

  return {

    get state () { return stateManager.getState() },

    addListener: stateManager.addListener,

    getString (key) {
      // In order to normalize messages across extensions and WebUI, replace all
      // chrome.i18n message placeholders with $N marker patterns. UI components
      // are responsible for replacing these markers with appropriate text or
      // using the markers to build HTML.
      return chrome.i18n.getMessage(key,
        ['$1', '$2', '$3', '$4', '$5', '$6', '$7', '$8', '$9'])
    },

    enableRewards () {
      chrome.braveRewards.enableRewards()
      // TODO: Do we need to refetch everything now?
      stateManager.update({ rewardsEnabled: true })
    },

    openRewardsSettings () {
      openTab('chrome://rewards')
    },

    refreshPublisherStatus () {
      const { publisherInfo } = stateManager.getState()
      if (!publisherInfo) {
        return
      }

      stateManager.update({ publisherRefreshing: true })
      console.log('refreshing publisher')

      chrome.braveRewards.refreshPublisher(publisherInfo.id, () => {
        updatePublisherInfo().then(() => {
          stateManager.update({ publisherRefreshing: false })
        }).catch(console.error)
      })
    },

    setIncludeInAutoContribute (include) {
      const { publisherInfo } = stateManager.getState()
      if (!publisherInfo) {
        return
      }

      // Confusingly, |includeInAutoContribution| takes an "exclude" parameter.
      chrome.braveRewards.includeInAutoContribution(publisherInfo.id, !include)

      stateManager.update({
        publisherInfo: {
          ...publisherInfo,
          autoContributeEnabled: include
        }
      })
    },

    setAutoContributeAmount (amount) {
      chrome.braveRewards.updatePrefs({ autoContributeAmount: amount })

      stateManager.update({
        settings: {
          ...stateManager.getState().settings,
          autoContributeAmount: amount
        }
      })
    },

    setAdsPerHour (adsPerHour) {
      chrome.braveRewards.updatePrefs({ adsPerHour })

      stateManager.update({
        settings: {
          ...stateManager.getState().settings,
          adsPerHour
        }
      })
    },

    hidePublisherUnverifiedNote () {
      writeLocalStorage('hide-publisher-unverified-note', true)
      stateManager.update({ hidePublisherUnverifiedNote: true })
    },

    sendTip () {
      getCurrentTabInfo().then((tabInfo) => {
        if (!tabInfo) {
          return
        }
        const { publisherInfo } = stateManager.getState()
        if (publisherInfo) {
          chrome.braveRewards.tipSite(tabInfo.id, publisherInfo.id, 'one-time')
          closePanel()
        }
      }).catch(console.error)
    },

    handleMonthlyTipAction (action) {
      getCurrentTabInfo().then((tabInfo) => {
        if (!tabInfo) {
          return
        }

        const tabId = tabInfo.id
        const { publisherInfo } = stateManager.getState()
        if (!publisherInfo) {
          return
        }

        const publisherId = publisherInfo.id

        switch (action) {
          case 'update':
            chrome.braveRewards.tipSite(tabId, publisherId, 'set-monthly')
            break
          case 'cancel':
            chrome.braveRewards.tipSite(tabId, publisherId, 'clear-monthly')
            break
        }

        closePanel()

      }).catch(console.error)
    },

    handleExternalWalletAction,

    handleNotificationAction (action) {
      switch (action.type) {
        case 'open-link':
          openTab((action as OpenLinkAction).url)
          break
        case 'backup-wallet':
          openTab('chrome://rewards#manage-wallet')
          break
        case 'claim-grant':
          loadCaptcha((action as ClaimGrantAction).grantId, 'pending')
          break
        case 'add-funds':
          handleExternalWalletAction('add-funds')
          break
        case 'reconnect-external-wallet':
          handleExternalWalletAction('reconnect')
          break
      }
    },

    dismissNotification (notification) {
      chrome.rewardsNotifications.deleteNotification(notification.id)
      const { notifications } = stateManager.getState()
      stateManager.update({
        notifications: notifications.filter(n => n.id !== notification.id)
      })
    },

    setNotificationsViewed () {
      const now = Date.now()
      stateManager.update({ notificationsLastViewed: now })
      writeLocalStorage('notifications-last-viewed', now)
    },

    solveGrantCaptcha (solution) {
      const { grantCaptchaInfo } = stateManager.getState()
      if (!grantCaptchaInfo) {
        return
      }

      function mapResult (result: number): GrantCaptchaStatus {
        switch (result) {
          case 0: return 'passed'
          case 6: return 'failed'
          default: return 'error'
        }
      }

      const json = JSON.stringify({
        captchaId: grantCaptchaInfo.id,
        x: Math.round(solution.x),
        y: Math.round(solution.y)
      })

      const grantId = grantCaptchaInfo.grantInfo.id

      chrome.braveRewards.attestPromotion(grantId, json, (result) => {
        const { grantCaptchaInfo } = stateManager.getState()
        if (!grantCaptchaInfo || grantCaptchaInfo.grantInfo.id !== grantId) {
          return
        }

        const status = mapResult(result)

        stateManager.update({
          grantCaptchaInfo: { ...grantCaptchaInfo, status }
        })

        // If the user failed the captcha, request a new captcha for the user.
        if (status === 'failed') {
          loadCaptcha(grantId, 'failed')
        }
      })
    },

    clearGrantCaptcha () {
      stateManager.update({ grantCaptchaInfo: null })
    }
  }
}
