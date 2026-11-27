#!/usr/bin/env bash
# Malvado 2; SPDX-License-Identifier: Apache-2.0
set -euo pipefail

cli=${1:?Expected the installed SDK CLI}
downloads=${2:?Expected the built example downloads directory}
root=$(cd "$(dirname "$0")/../.." && pwd)
export LIBGL_ALWAYS_SOFTWARE=1
xvfb-run -a -s '-screen 0 1280x1024x24' bash -euc '
  openbox &
  trap "kill $!" EXIT
  timeout 10 bash -c "until wmctrl -m >/dev/null 2>&1; do sleep 0.1; done"
  (
    cd "$1"
    MALVADO_INVENTORY_ASSETS=1 MALVADO_INVENTORY_WINDOWED=1 \
      timeout 15 "$2" run tests/lua_module_inventory.lua
  )
  python3 "$1/tests/docs_examples.py" "$2" "$3"
' bash "$root" "$(realpath "$cli")" "$(realpath "$downloads")"
