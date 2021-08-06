import * as React from 'react'
import { UserAccountType } from '../../../constants/types'
import { reduceAddress } from '../../../utils/reduce-address'
import { create } from 'ethereum-blockies'
// Styled Components
import {
  StyledWrapper,
  AccountAddress,
  AccountAndAddress,
  AccountCircle,
  AccountName
} from './style'

export interface Props {
  account: UserAccountType
  onSelectAccount: () => void
}

function SelectAccountItem (props: Props) {
  const { account, onSelectAccount } = props

  const orb = React.useMemo(() => {
    return create({ seed: account.address, size: 8, scale: 16 }).toDataURL()
  }, [account])

  return (
    <StyledWrapper onClick={onSelectAccount}>
      <AccountCircle orb={orb} />
      <AccountAndAddress>
        <AccountName>{account.name}</AccountName>
        <AccountAddress>{reduceAddress(account.address)}</AccountAddress>
      </AccountAndAddress>
    </StyledWrapper>
  )
}

export default SelectAccountItem