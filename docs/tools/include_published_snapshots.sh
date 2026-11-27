#!/usr/bin/env bash
# Malvado 2; SPDX-License-Identifier: Apache-2.0
set -euo pipefail

site=${1:?Expected the built site directory}
test -s "$site/index.html"
tools=$(cd "$(dirname "$0")" && pwd)
versions=$(python3 "$tools/../../tools/release_notes.py" snapshots)
bundles=$(mktemp -d "${RUNNER_TEMP:-${TMPDIR:-/tmp}}/malvado-docs.XXXXXX")
trap 'rm -rf "$bundles"' EXIT
while IFS= read -r version; do
  [ -n "$version" ] || continue
  bundle="malvado-$version-release-bundle.tar.gz"
  gh release download "v$version" --repo jepemo/malvado --pattern "$bundle" --dir "$bundles"
  tar -xzf "$bundles/$bundle" -C "$site" --strip-components=2 "./docs/$version/"
  test -s "$site/$version/index.html"
  python3 "$tools/archived_install.py" "$site/$version" "$version"
done <<< "$versions"
