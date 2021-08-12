/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

const css = `
  button, input, select, textarea {
    color: inherit;
    font-family: inherit;
    font-size: inherit;
    line-height: inherit;
  }

  svg {
    fill: currentColor;
  }

  body {
    margin: 0;
  }

  * {
    box-sizing: border-box;
  }
`

export function insertCSSReset () {
  document.head.insertAdjacentHTML('beforeend', `<style>${css}</style>`)
}
