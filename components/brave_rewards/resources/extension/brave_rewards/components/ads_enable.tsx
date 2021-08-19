/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import { bindActionCreators, Dispatch } from 'redux'
import { connect } from 'react-redux'

import { BraveTalkOptInForm } from '../../../shared/components/onboarding/brave_talk_opt_in_form'

import * as rewardsPanelActions from '../actions/rewards_panel_actions'

const rewardsTourURL = 'brave_rewards_panel.html#tour'

interface Props extends RewardsExtension.ComponentProps {
}

interface State {
}

export class AdsEnablePanel extends React.Component<Props, State> {
  constructor (props: Props) {
    super(props)
  }

  componentDidMount () {
    chrome.braveRewards.isInitialized((initialized: boolean) => {
      if (initialized) {
        this.props.actions.initialized()
        this.startAdsEnable()
      }
    })
  }

  componentDidUpdate (prevProps: Props, prevState: State) {
    if (prevProps.rewardsPanelData.initializing &&
        !this.props.rewardsPanelData.initializing) {
      this.startAdsEnable()
    }
  }

  startAdsEnable = () => {
    chrome.braveRewards.shouldShowOnboarding((showOnboarding: boolean) => {
      this.props.actions.onShouldShowOnboarding(showOnboarding)
    })
  }

  render () {
    const onEnable = () => {
      chrome.braveRewards.enableRewards()
    }

    const onTakeTour = () => {
      window.location.href = rewardsTourURL
    }

    return (
      <BraveTalkOptInForm
        rewardsEnabled={!this.props.rewardsPanelData.showOnboarding}
        onEnable={onEnable}
        onTakeTour={onTakeTour}
      />
    )
  }
}

export const mapStateToProps = (state: RewardsExtension.ApplicationState) => ({
  rewardsPanelData: state.rewardsPanelData
})

export const mapDispatchToProps = (dispatch: Dispatch) => ({
  actions: bindActionCreators(rewardsPanelActions, dispatch)
})

export default connect(
  mapStateToProps,
  mapDispatchToProps
)(AdsEnablePanel)
