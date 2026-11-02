#!/bin/sh
# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

set -eu

if [ "$#" -ne 1 ] || [ ! -f "$1" ] || [ ! -f "$1.sha256" ]; then
    printf 'Usage: %s SDK-ARCHIVE.tar.gz\nThe archive and its .sha256 file must exist.\n' "$0" >&2
    exit 2
fi

source_dir=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd -P)
archive_dir=$(CDPATH= cd -- "$(dirname "$1")" && pwd -P)
archive_name=$(basename "$1")
work_dir=$(mktemp -d /tmp/malvado-linux-check.XXXXXX)
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

printf 'Create and export games in a fresh Ubuntu container.\n'
docker run --rm --platform linux/amd64 \
    -v "$source_dir:/source:ro" -v "$archive_dir:/candidate:ro" -v "$work_dir:/work" \
    -e "ARCHIVE_NAME=$archive_name" ubuntu:24.04 bash -ec '
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y -qq cmake build-essential ninja-build libx11-dev libxrandr-dev \
          libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev libgl1-mesa-dri xvfb xauth
        Xvfb :99 -screen 0 1280x1024x24 -nolisten tcp >/tmp/xvfb.log 2>&1 &
        xvfb_pid=$!
        trap "kill $xvfb_pid" EXIT
        sleep 2
        export DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1
        cmake "-DARCHIVE=/candidate/$ARCHIVE_NAME" -DSOURCE=/source -DWORK=/work/games \
          -DVISUAL=ON -DKEEP_OUTPUT=ON -P /source/tests/sdk_clean.cmake
    '

printf 'Run exported games in a separate player container.\n'
docker run --rm --platform linux/amd64 -v "$work_dir/games:/games:ro" ubuntu:24.04 bash -ec '
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y -qq libgl1 libgl1-mesa-dri libx11-6 libxrandr2 \
          libxinerama1 libxcursor1 libxi6 xvfb xauth
        ! command -v cc
        ! command -v cmake
        ! command -v lua
        test ! -e /games/sdk
        Xvfb :99 -screen 0 1280x1024x24 -nolisten tcp >/tmp/xvfb.log 2>&1 &
        xvfb_pid=$!
        trap "kill $xvfb_pid" EXIT
        sleep 2
        export DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1
        cd /
        "/games/export_game/exported game/game"
        "/games/native_cli/C game/dist/game/game"
        "/games/C graphical game/dist/game/game"
    '
printf 'Linux candidate player check passed.\n'
