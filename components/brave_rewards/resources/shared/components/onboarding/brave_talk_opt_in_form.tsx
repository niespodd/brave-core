/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { LocaleContext } from '../../lib/locale_context'
import { TermsOfService } from '../terms_of_service'
import { MainButton } from './main_button'

import { CheckCircleIcon } from './icons/check_circle'
import { CashbackIcon } from './icons/cashback'

import * as style from './brave_talk_opt_in_form.style'

interface Props {
  showRewardsOnboarding: boolean
  onEnable: () => void
}

export function BraveTalkOptInForm (props: Props) {
  const { getString } = React.useContext(LocaleContext)
  const [showStartFreeCall, setShowStartFreeCall] = React.useState(false)

  const onRewardsTurnedOn = () => {
    props.onEnable()
    setShowStartFreeCall(true)
  }

  if (showStartFreeCall) {
    return (
      <style.root>
        <CheckCircleIcon />
        <style.header>
          {getString('braveTalkCanStartFreeCall')}
        </style.header>
        <style.text>
          {getString('braveTalkClickAnywhereToBraveTalk')}
        </style.text>
        <style.terms>
          <TermsOfService />
        </style.terms>
      </style.root>
    )
  } else if (props.showRewardsOnboarding) {
    return (
      <style.root>
        <CashbackIcon />
        <style.header>
          {getString('braveTalkTurnOnRewardsToStartCall')}
        </style.header>
        <style.text>
          {getString('braveTalkBraveRewardsDescription')}
        </style.text>
        <style.enable>
          <MainButton onClick={onRewardsTurnedOn}>
            {getString('braveTalkTurnOnRewards')}
          </MainButton>
        </style.enable>
        <style.terms>
          <TermsOfService />
        </style.terms>
      </style.root>
    )
  }

  return (
    <style.root>
      <CashbackIcon />
      <style.header>
        {getString('braveTalkTurnOnPrivateAdsToStartCall')}
      </style.header>
      <style.text>
        {getString('braveTalkPrivateAdsDescription')}
      </style.text>
      <style.enable>
        <MainButton onClick={onRewardsTurnedOn}>
          {getString('braveTalkTurnOnPrivateAds')}
        </MainButton>
      </style.enable>
      <style.terms>
        <TermsOfService />
      </style.terms>
    </style.root>
  )
}
