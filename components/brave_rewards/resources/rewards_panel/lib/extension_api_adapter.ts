/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Store } from 'webext-redux'

import { Notification } from '../../shared/components/notifications'
import { RewardsSummaryData } from '../../shared/components/wallet_card'
import { mapNotification } from './notification_adapter'

import {
  ExternalWallet,
  ExternalWalletProvider,
  ExternalWalletStatus
} from '../../shared/lib/external_wallet'

import {
  EarningsInfo,
  ExchangeInfo,
  GrantInfo,
  Options,
  PublisherInfo,
  Settings
} from './interfaces'

export function getRewardsBalance () {
  return new Promise<number>((resolve) => {
    chrome.braveRewards.fetchBalance((balance) => { resolve(balance.total) })
  })
}

export function getSettings () {
  return new Promise<Settings>((resolve) => {
    chrome.braveRewards.getPrefs((prefs) => {
      resolve({
        adsPerHour: prefs.adsPerHour,
        autoContributeAmount: prefs.autoContributeAmount
      })
    })
  })
}

function mapWalletType (type: string): ExternalWalletProvider | null {
  switch (type) {
    case 'bitflyer': return 'bitflyer'
    case 'uphold': return 'uphold'
    case 'gemini': return 'gemini'
    default: return null
  }
}

export function getExternalWalletProviders () {
  return new Promise<ExternalWalletProvider[]>((resolve) => {
    // TODO(zenparsing): The extension API currently does not support retrieving
    // an external wallet provider list.
    chrome.braveRewards.getExternalWallet((result, wallet) => {
      const provider = mapWalletType(wallet.type)
      resolve(provider ? [provider] : [])
    })
  })
}

export function getEarningsInfo () {
  return new Promise<EarningsInfo | null>((resolve) => {
    chrome.braveRewards.getAdsAccountStatement((success, statement) => {
      if (success) {
        resolve({
          earningsLastMonth: statement.earningsLastMonth,
          earningsThisMonth: statement.earningsThisMonth,
          nextPaymentDate: statement.nextPaymentDate
        })
      } else {
        resolve(null)
      }
    })
  })
}

export function getRewardsParameters () {
  interface Result {
    exchangeInfo: ExchangeInfo
    options: Options
  }

  return new Promise<Result>((resolve) => {
    chrome.braveRewards.getRewardsParameters((parameters) => {
      resolve({
        options: {
          autoContributeAmounts: parameters.autoContributeChoices
        },
        exchangeInfo: {
          currency: 'USD',
          rate: parameters.rate
        }
      })
    })
  })
}

const externalWalletLoginURLs = new Map<ExternalWalletProvider, string>()

export function getExternalWalletLoginURL (provider: ExternalWalletProvider) {
  return new Promise<string>((resolve) => {
    resolve(externalWalletLoginURLs.get(provider) || '')
  })
}

export function getExternalWallet () {
  function mapStatus (status: number): ExternalWalletStatus | null {
    switch (status) {
      case 1: // CONNECTED
      case 2: // VERIFIED
        return 'verified'
      case 3: // DISCONNECTED_NOT_VERIFIED
      case 4: // DISCONNECTED_VERIFIED
        return 'disconnected'
      case 5: // PENDING
        return 'pending'
    }
    return null
  }

  return new Promise<ExternalWallet | null>((resolve) => {
    chrome.braveRewards.getExternalWallet((result, wallet) => {
      // TODO: Why does this even report when the access token is expired?
      // Access token expired: result === 24

      const provider = mapWalletType(wallet.type)
      const status = mapStatus(wallet.status)

      if (!provider || !status) {
        resolve(null)
      } else {
        externalWalletLoginURLs.set(provider, wallet.loginUrl)

        resolve({
          provider,
          status,
          username: wallet.userName,
          links: {
            account: wallet.accountUrl,
            addFunds: wallet.addUrl,
            completeVerification: wallet.verifyUrl
          }
        })
      }
    })
  })
}

export function getRewardsSummaryData () {
  return new Promise<RewardsSummaryData>((resolve) => {
    const now = new Date()
    const month = now.getMonth() + 1
    const year = now.getFullYear()

    chrome.braveRewards.getBalanceReport(month, year, (balanceReport) => {
      resolve({
        grantClaims: balanceReport.grant,
        adEarnings: balanceReport.ads,
        autoContributions: balanceReport.contribute,
        oneTimeTips: balanceReport.tips,
        monthlyTips: balanceReport.monthly
      })
    })
  })
}

export function getNotifications () {
  return new Promise<Notification[]>((resolve) => {
    chrome.braveRewards.getAllNotifications((list) => {
      const notifications: Notification[] = []
      for (const obj of list) {
        const notification = mapNotification(obj)
        if (notification) {
          notifications.push(notification)
        }
      }
      resolve(notifications)
    })
  })
}

export function onGrantsUpdated (callback: (grants: GrantInfo[]) => void) {
  chrome.braveRewards.onPromotions.addListener((result, promotions) => {
    if (result === 1) { // Error
      return
    }

    const grants: GrantInfo[] = []
    for (const obj of promotions) {
      const source = obj.type === 1 ? 'ads' : 'ugp'
      grants.push({
        id: obj.promotionId,
        source,
        amount: obj.amount,
        expiresAt: obj.expiresAt * 1000 || null
      })
    }

    callback(grants)
  })

  chrome.braveRewards.fetchPromotions()
}

export function getRewardsEnabled () {
  return new Promise<boolean>((resolve) => {
    // TODO(zenparsing): Add a method to get the rewards enabled status directly
    // instead of inferring it through |showOnboarding|.
    chrome.braveRewards.shouldShowOnboarding((showOnboarding) => {
      resolve(!showOnboarding)
    })
  })
}

const backgroundReduxStore = new Store({ portName: 'REWARDSPANEL' })

function getMonthlyTipAmount (publisherKey: string) {
  return new Promise<number>((resolve) => {
    chrome.braveRewards.getRecurringTips((result) => {
      for (const item of result.recurringTips) {
        if (item.publisherKey === publisherKey) {
          resolve(item.amount)
          return
        }
      }
      resolve(0)
    })
  })
}

function getPublisherFromBackgroundState (tabId: number): any {
  const state = backgroundReduxStore.getState()
  if (!state) {
    return {}
  }
  const { rewardsPanelData } = state
  if (!rewardsPanelData) {
    return {}
  }
  const { publishers } = rewardsPanelData
  if (!publishers) {
    return {}
  }
  return publishers[`key_${tabId}`] || {}
}

export async function getPublisherInfo (tabId: number) {
  await backgroundReduxStore.ready()

  const publisher = getPublisherFromBackgroundState(tabId)
  const { publisherKey } = publisher
  if (!publisherKey || typeof publisherKey !== 'string') {
    return null
  }

  const supportedWalletProviders: ExternalWalletProvider[] = []
  let registered = true

  switch (Number(publisher.status) || 0) {
    case 0: // NOT_VERIFIED
      registered = false
      break
    case 2: // UPHOLD_VERIFIED
      supportedWalletProviders.push('uphold')
      break
    case 3: // BITFLYER_VERIFIED
      supportedWalletProviders.push('bitflyer')
      break
    case 4: // GEMINI_VERIFIED
      supportedWalletProviders.push('gemini')
      break
  }

  const info: PublisherInfo = {
    id: publisherKey,
    name: String(publisher.name || ''),
    icon: String(publisher.favIconUrl || ''),
    registered,
    attentionScore: Number(publisher.percentage) / 100 || 0,
    autoContributeEnabled: !publisher.excluded,
    monthlyContribution: await getMonthlyTipAmount(publisherKey),
    supportedWalletProviders
  }

  return info
}
