#!/usr/bin/env bash
# Malvado 2; SPDX-License-Identifier: Apache-2.0
set -eo pipefail

if [[ $# -lt 3 || $# -gt 4 || ( "$2" != Debug && "$2" != Release ) ]]; then
    echo 'Usage: verify-android-exports.sh ANDROID_SDK Debug|Release NEW_WORK_DIR [EMULATOR_SERIAL]' >&2
    exit 2
fi

sdk=$(cd "$1" && pwd)
config=$2
mkdir "$3"
work=$(cd "$3" && pwd)
source=$(cd "$(dirname "$0")/.." && pwd)
serial=${4:-}
runner=(--package-only)
if [[ -n "$serial" ]]; then
    runner=(--serial "$serial")
    if [[ ! "$serial" =~ ^emulator-[0-9]+$ ]]; then
        echo "Select an explicit emulator serial" >&2
        exit 2
    fi
    adb=("$sdk/platform-tools/adb" -s "$serial")
    api=$("${adb[@]}" shell getprop ro.build.version.sdk | tr -d '\r')
    if [[ "$api" != 34 ]]; then
        echo "Android export verification requires an API 34 emulator, found: $api" >&2
        exit 1
    fi
    abi=$("${adb[@]}" shell getprop ro.product.cpu.abi | tr -d '\r')
    case "$abi" in
        arm64-v8a) ;;
        x86_64)
            abi_list=$("${adb[@]}" shell getprop ro.product.cpu.abilist | tr -d '\r')
            if [[ ",$abi_list," != *,arm64-v8a,* ]]; then
                echo "The selected x86_64 emulator does not advertise arm64-v8a translation" >&2
                exit 1
            fi
            bridge_exec=$("${adb[@]}" shell getprop ro.enable.native.bridge.exec | tr -d '\r')
            if [[ "$bridge_exec" != 1 ]]; then
                echo "The selected x86_64 emulator does not enable translated executables for the Rust ABI probes" >&2
                exit 1
            fi
            echo "Verifying arm64 APKs and Rust ABI probes through Android native translation ($abi_list)"
            ;;
        *)
            echo "Unsupported Android emulator ABI: $abi" >&2
            exit 1
            ;;
    esac
fi

cmake -S "$source" -B "$work/host" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$work/host" --parallel
cmake --install "$work/host" --prefix "$work/installed cli"

target_args=(
    "-DCMAKE_TOOLCHAIN_FILE=$sdk/ndk/27.2.12479018/build/cmake/android.toolchain.cmake"
    -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-28
    "-DCMAKE_BUILD_TYPE=$config" -DCMAKE_INSTALL_LIBDIR=lib
    -DMALVADO_BUILD_RAYLIB=ON -DMALVADO_BUILD_ASSETS=ON
)
cmake -S "$source" -B "$work/lua" "${target_args[@]}" -DMALVADO_BUILD_LUA=ON
cmake --build "$work/lua" --target malvado-android-game --parallel
cmake --install "$work/lua" --component AndroidRuntime --prefix "$work/lua runtime"
cmake -S "$source" -B "$work/native" "${target_args[@]}" -DMALVADO_BUILD_LUA=OFF
cmake --build "$work/native" --parallel
cmake --install "$work/native" --prefix "$work/native sdk"

common=(--cli "$work/installed cli/bin/malvado" --android-sdk "$sdk" "${runner[@]}")
profile=()
signing=()
formats=(apk)
if [[ "$config" == Release ]]; then
    keys=$(mktemp -d "$work/.signing.XXXXXXXX")
    trap 'rm -rf "$keys"' EXIT
    export MALVADO_TEST_STORE_PASSWORD=android
    keytool -genkeypair -keystore "$keys/test.p12" -storepass:env MALVADO_TEST_STORE_PASSWORD \
        -alias test -keyalg RSA -validity 30 -dname 'CN=Malvado test only'
    profile=(--release)
    signing=(--keystore "$keys/test.p12" --key-alias test --store-password-env MALVADO_TEST_STORE_PASSWORD)
    formats+=(aab)
    python3 "$source/tools/android_bundle.py" --output "$work/bundle-tools"
fi

for format in "${formats[@]}"; do
    bundle=()
    if [[ "$format" == aab ]]; then
        bundle=(--bundle-tools "$work/bundle-tools")
    fi
    python3 "$source/tools/test_android_export.py" "${common[@]}" "${profile[@]}" "${bundle[@]}" \
        --format "$format" --runtime-sdk "$work/lua runtime" --work "$work/lua-$format"
    python3 "$source/tools/test_android_c_export.py" "${common[@]}" "${profile[@]}" "${signing[@]}" "${bundle[@]}" \
        --format "$format" --runtime-sdk "$work/native sdk" --work "$work/c-$format"
    if [[ -n "$serial" ]]; then
        python3 "$source/tools/test_android_rust_export.py" "${common[@]}" "${profile[@]}" "${signing[@]}" "${bundle[@]}" \
            --format "$format" --runtime-sdk "$work/native sdk" --work "$work/rust-$format"
    fi
done

if [[ -n "$serial" ]]; then
    python3 "$source/tools/test_android_c_export.py" "${common[@]}" "${profile[@]}" "${signing[@]}" \
        --runtime-sdk "$work/native sdk" --work "$work/c-audio-focus" --audio-focus
fi
