/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { ExternalWalletProvider } from '../../shared/lib/external_wallet'

import {
  Notification,
  AutoContributeCompletedNotification,
  MonthlyContributionFailedNotification,
  GrantAvailableNotification,
  PendingPublisherVerifiedNotification,
  PendingTipFailedNotification,
  ExternalWalletVerifiedNotification,
  ExternalWalletDisconnectedNotification,
  ExternalWalletLinkingFailedNotification
} from '../../shared/components/notifications'

function parseGrantId (id: string) {
  const parts = id.split('_')
  return (parts.length > 1 && parts.pop()) || ''
}

function mapProvider (name: string): ExternalWalletProvider | null {
  switch (name.toLowerCase()) {
    case 'uphold': return 'uphold'
    case 'bitflyer': return 'bitflyer'
    case 'gemini': return 'gemini'
  }
  return null
}

enum ExtensionNotificationType {
  AUTO_CONTRIBUTE = 1,
  GRANT = 2,
  GRANT_ADS = 3,
  INSUFFICIENT_FUNDS = 6,
  BACKUP_WALLET = 7,
  TIPS_PROCESSED = 8,
  VERIFIED_PUBLISHER = 10,
  PENDING_NOT_ENOUGH_FUNDS = 11,
  GENERAL_LEDGER = 12
}

export function mapNotification (
  obj: RewardsExtension.Notification
): Notification | null {
  switch (obj.type) {
    case ExtensionNotificationType.AUTO_CONTRIBUTE:
      switch (parseInt(obj.args[1], 10)) {
        case 0: // Success
          return {
            id: obj.id,
            timeStamp: obj.timestamp,
            type: 'auto-contribute-completed',
            amount: parseFloat(obj.args[3]) || 0
          } as AutoContributeCompletedNotification
        case 1: // General error
          return {
            id: obj.id,
            timeStamp: obj.timestamp,
            type: 'monthly-contribution-failed',
            reason: 'unknown'
          } as MonthlyContributionFailedNotification
        case 15: // Not enough funds
          return {
            id: obj.id,
            timeStamp: obj.timestamp,
            type: 'monthly-contribution-failed',
            reason: 'insufficient-funds'
          } as MonthlyContributionFailedNotification
      }
      break
    case ExtensionNotificationType.GRANT:
      return {
        id: obj.id,
        timeStamp: obj.timestamp,
        type: 'grant-available',
        source: 'ugp',
        grantId: parseGrantId(obj.id)
      } as GrantAvailableNotification
    case ExtensionNotificationType.GRANT_ADS:
      return {
        id: obj.id,
        timeStamp: obj.timestamp,
        type: 'grant-available',
        source: 'ads',
        grantId: parseGrantId(obj.id)
      } as GrantAvailableNotification
    case ExtensionNotificationType.BACKUP_WALLET:
      return {
        id: obj.id,
        timeStamp: obj.timestamp,
        type: 'backup-wallet'
      }
    case ExtensionNotificationType.INSUFFICIENT_FUNDS:
      return {
        id: obj.id,
        timeStamp: obj.timestamp,
        type: 'add-funds'
      }
    case ExtensionNotificationType.TIPS_PROCESSED:
      return {
        id: obj.id,
        timeStamp: obj.timestamp,
        type: 'monthly-tip-completed'
      }
    case ExtensionNotificationType.VERIFIED_PUBLISHER:
      return {
        id: obj.id,
        timeStamp: obj.timestamp,
        type: 'pending-publisher-verified',
        publisherName: obj.args[0] || ''
      } as PendingPublisherVerifiedNotification
    case ExtensionNotificationType.PENDING_NOT_ENOUGH_FUNDS:
      return {
        id: obj.id,
        timeStamp: obj.timestamp,
        type: 'pending-tip-failed',
        reason: 'insufficient-funds'
      } as PendingTipFailedNotification
    case ExtensionNotificationType.GENERAL_LEDGER:
      switch (obj.args[0]) {
        case 'wallet_device_limit_reached':
          return {
            type: 'external-wallet-linking-failed',
            provider: 'uphold', // TODO(zenparsing): We need to add this.
            reason: 'device-limit-reached'
          } as ExternalWalletLinkingFailedNotification
        case 'wallet_disconnected':
          return {
            type: 'external-wallet-disconnected',
            provider: 'uphold' // TODO(zenparsing): Do we have this?
          } as ExternalWalletDisconnectedNotification
        case 'wallet_mismatched_provider_accounts':
          return {
            type: 'external-wallet-linking-failed',
            provider: mapProvider(obj.args[1] || '') || 'uphold', // TODO: ?
            reason: 'mismatched-provider-accounts'
          } as ExternalWalletLinkingFailedNotification
        case 'wallet_new_verified':
          return {
            type: 'external-wallet-verified',
            provider: mapProvider(obj.args[1] || '') || 'uphold' // TODO: ?
          } as ExternalWalletVerifiedNotification
        case 'uphold_bat_not_allowed_for_user':
          return {
            type: 'external-wallet-linking-failed',
            provider: 'uphold',
            reason: 'uphold-bat-not-supported'
          } as ExternalWalletLinkingFailedNotification
        case 'uphold_blocked_user':
          return {
            type: 'external-wallet-linking-failed',
            provider: 'uphold',
            reason: 'uphold-user-blocked'
          } as ExternalWalletLinkingFailedNotification
        case 'uphold_pending_user':
          return {
            type: 'external-wallet-linking-failed',
            provider: 'uphold',
            reason: 'uphold-user-pending'
          } as ExternalWalletLinkingFailedNotification
        case 'uphold_restricted_user':
          return {
            type: 'external-wallet-linking-failed',
            provider: 'uphold',
            reason: 'uphold-user-restricted'
          } as ExternalWalletLinkingFailedNotification
      }
      break
  }
  return null
}
