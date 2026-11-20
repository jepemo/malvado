#!/usr/bin/env bash
# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0
#
# Malvado SDK installer for macOS and Linux.
#
# This script downloads a released SDK archive from GitHub Releases, verifies
# its SHA-256 checksum, and installs it into a user-writable prefix. It does
# not build from source and does not require a C toolchain.
#
# Running a remote script through a shell executes downloaded code. If you
# prefer to inspect it first:
#
#   curl -fsSL https://malvado.dev/install.sh -o install.sh
#   less install.sh
#   bash install.sh
#
# Usage: see `bash install.sh --help`.

set -euo pipefail

REPO="jepemo/malvado"
PROJECT="malvado"
DEFAULT_PREFIX="${XDG_DATA_HOME:-$HOME/.local/share}/${PROJECT}"
MARKER=".malvado-install"
API="https://api.github.com/repos/${REPO}"
DOWNLOAD_BASE="https://github.com/${REPO}/releases/download"

VERSION=""
PREFIX=""
UNINSTALL=0
ADD_TO_PATH=0
INCLUDE_PRERELEASE=0

if [ -t 1 ]; then
  C_INFO="\033[1;34m"; C_ERR="\033[1;31m"; C_OK="\033[1;32m"; C_OFF="\033[0m"
else
  C_INFO=""; C_ERR=""; C_OK=""; C_OFF=""
fi

say()  { printf "${C_INFO}==>${C_OFF} %s\n" "$*"; }
info() { printf "${C_INFO}  ->${C_OFF} %s\n" "$*"; }
ok()   { printf "${C_OK}  ok${C_OFF} %s\n" "$*"; }
err()  { printf "${C_ERR}error:${C_OFF} %s\n" "$*" >&2; }
die()  { err "$*"; exit 1; }

usage() {
  cat <<EOF
Malvado SDK installer

Usage:
  curl -fsSL https://malvado.dev/install.sh | bash
  curl -fsSL https://malvado.dev/install.sh | bash -s -- [options]

Options:
  --version VERSION   Install a specific Malvado 2.x version (for example
                      2.0.0 or 2.0.0-beta.1). Default: latest stable release.
  --prerelease        When resolving the latest version, allow prereleases.
  --prefix DIR        Install prefix (default: \$XDG_DATA_HOME/malvado, or
                      \$HOME/.local/share/malvado).
  --add-to-path       Append the SDK bin directory to your shell rc file.
                      Without this flag the installer only prints the change.
  --uninstall         Remove an installation created by this installer.
  --help, -h          Show this help.

The SDK is installed as a whole prefix (bin/, lib/, include/, share/). The
installer verifies the release checksum before extraction and refuses to
overwrite a directory that it did not create. No sudo is required.

Examples:
  curl -fsSL https://malvado.dev/install.sh | bash
  curl -fsSL https://malvado.dev/install.sh | bash -s -- --version 2.0.0-beta.1
  curl -fsSL https://malvado.dev/install.sh | bash -s -- --prefix ~/malvado
EOF
}

parse_args() {
  while [ $# -gt 0 ]; do
    case "$1" in
      --help|-h) usage; exit 0 ;;
      --version) [ $# -ge 2 ] || die "--version needs a value"; VERSION="$2"; shift ;;
      --prefix)  [ $# -ge 2 ] || die "--prefix needs a value";  PREFIX="$2";  shift ;;
      --prerelease) INCLUDE_PRERELEASE=1 ;;
      --add-to-path) ADD_TO_PATH=1 ;;
      --uninstall) UNINSTALL=1 ;;
      *) die "unknown option: $1 (use --help)" ;;
    esac
    shift
  done
}

require_tools() {
  command -v tar >/dev/null 2>&1 || die "tar is required"
  if command -v curl >/dev/null 2>&1; then
    DOWNLOADER="curl"
  elif command -v wget >/dev/null 2>&1; then
    DOWNLOADER="wget"
  else
    die "curl or wget is required"
  fi
  if command -v sha256sum >/dev/null 2>&1; then
    SHACMD="sha256sum"
  elif command -v shasum >/dev/null 2>&1; then
    SHACMD="shasum"
  elif command -v openssl >/dev/null 2>&1; then
    SHACMD="openssl"
  else
    die "a SHA-256 tool (sha256sum, shasum or openssl) is required"
  fi
}

download() {
  # download URL OUTPUT
  if [ "$DOWNLOADER" = "curl" ]; then
    curl -fL --proto '=https' --tlsv1.2 -sS -o "$2" "$1"
  else
    wget -q -O "$2" "$1"
  fi
}

sha256_of() {
  case "$SHACMD" in
    sha256sum) sha256sum "$1" | awk '{print $1}' ;;
    shasum)    shasum -a 256 "$1" | awk '{print $1}' ;;
    openssl)   openssl dgst -sha256 "$1" | awk '{print $NF}' ;;
  esac
}

detect_platform() {
  os="$(uname -s 2>/dev/null || echo unknown)"
  arch="$(uname -m 2>/dev/null || echo unknown)"
  case "$os" in
    Darwin) OS="darwin" ;;
    Linux)  OS="linux" ;;
    *) die "unsupported operating system: ${os}. This installer supports macOS and Linux. On Windows, see the SDK downloads at https://github.com/${REPO}/releases" ;;
  esac
  case "$arch" in
    arm64|aarch64) ARCH="arm64" ;;
    x86_64|amd64)  ARCH="x86_64" ;;
    *) ARCH="$arch" ;;
  esac
  if [ "$OS" = "darwin" ] && [ "$ARCH" = "arm64" ]; then
    PLATFORM="darwin-arm64"
  elif [ "$OS" = "linux" ] && [ "$ARCH" = "x86_64" ]; then
    PLATFORM="linux-x86_64"
  else
    die "unsupported platform: ${OS}/${ARCH}. The validated desktop SDKs are macOS arm64 and Linux x86_64. See https://github.com/${REPO}/releases"
  fi
}

api_get() {
  download "$1" "$2" || return 1
}

# Resolve VERSION (no leading v) and TAG (with leading v).
resolve_version() {
  if [ -z "$VERSION" ] || [ "$VERSION" = "latest" ]; then
    say "Resolving the latest ${PROJECT} release"
    tmp="$(mktemp)"
    api_get "${API}/releases?per_page=100" "$tmp" || die "cannot reach the GitHub API"
    TAG="$(grep -oE '"tag_name": *"[^"]*"|"prerelease": *(true|false)' "$tmp" | awk -v inc="$INCLUDE_PRERELEASE" '
      /^"tag_name"/   { t=$0; sub(/.*: *"/,"",t); sub(/"$/,"",t) }
      /^"prerelease"/ { p=$0; sub(/.*: */,"",p); if ((inc=="1" || p=="false") && t ~ /^v?2\.[0-9]+\.[0-9]+$/) { print t; exit } }')"
    rm -f "$tmp"
    [ -n "$TAG" ] || die "no stable Malvado 2.x release is published yet. Install a specific prerelease with: --version 2.0.0-beta.1"
    VERSION="${TAG#v}"
    return
  fi

  VERSION="${VERSION#v}"
  case "$VERSION" in
    2.*) : ;;
    *) die "this installer installs Malvado 2.x SDKs; '${VERSION}' is not supported" ;;
  esac
  # Confirm the tag exists, with or without the v prefix.
  for candidate in "v${VERSION}" "${VERSION}"; do
    tmp="$(mktemp)"
    if api_get "${API}/releases/tags/${candidate}" "$tmp" >/dev/null 2>&1; then
      TAG="$candidate"; rm -f "$tmp"; return
    fi
    rm -f "$tmp"
  done
  die "release '${VERSION}' was not found at https://github.com/${REPO}/releases"
}

default_prefix() {
  if [ -n "$PREFIX" ]; then
    printf '%s' "$PREFIX"
  else
    printf '%s' "$DEFAULT_PREFIX"
  fi
}

expand_prefix() {
  # Expand a leading ~ and make the path absolute.
  case "$1" in
    "~")   printf '%s' "$HOME" ;;
    "~/"*) printf '%s/%s' "$HOME" "${1#\~/}" ;;
    /*)    printf '%s' "$1" ;;
    *)     printf '%s/%s' "$PWD" "$1" ;;
  esac
}

do_uninstall() {
  prefix="$(expand_prefix "$(default_prefix)")"
  if [ ! -d "$prefix" ]; then
    say "No Malvado installation found at $prefix"
    return 0
  fi
  if [ ! -f "$prefix/$MARKER" ]; then
    die "$prefix was not created by this installer; refusing to remove it"
  fi
  say "Removing $prefix"
  rm -rf "$prefix"
  ok "Uninstalled"
  info "If you added a PATH line for $prefix/bin, remove it from your shell rc file."
}

install_prefix() {
  # install_prefix EXTRACT_DIR
  extract="$1"
  prefix="$(expand_prefix "$(default_prefix)")"
  parent="$(dirname "$prefix")"
  mkdir -p "$parent" || die "cannot create $parent"

  if [ -e "$prefix" ] && [ ! -f "$prefix/$MARKER" ]; then
    die "$prefix already exists and was not created by this installer. Choose another prefix with --prefix."
  fi

  staging="${prefix}.new.$$"
  backup="${prefix}.old.$$"
  rm -rf "$staging"
  mv "$extract" "$staging" || die "cannot stage the extracted SDK"
  {
    printf 'version=%s\n' "$VERSION"
    printf 'platform=%s\n' "$PLATFORM"
  } > "$staging/$MARKER"

  if [ -e "$prefix" ]; then
    mv "$prefix" "$backup" || { rm -rf "$staging"; die "cannot move the previous installation"; }
  fi
  if ! mv "$staging" "$prefix"; then
    [ -e "$backup" ] && mv "$backup" "$prefix"
    die "cannot move the new SDK into $prefix"
  fi

  if "$prefix/bin/$PROJECT" --version >/dev/null 2>&1; then
    [ -e "$backup" ] && rm -rf "$backup"
    ok "Installed to $prefix"
  else
    rm -rf "$prefix"
    [ -e "$backup" ] && mv "$backup" "$prefix"
    die "the installed CLI failed to run; previous installation restored"
  fi
}

maybe_add_to_path() {
  bindir="$1"
  case ":$PATH:" in
    *":$bindir:"*) return ;;
  esac
  if [ "$ADD_TO_PATH" = "1" ]; then
    shell_name="$(basename "${SHELL:-sh}")"
    case "$shell_name" in
      zsh) rc="$HOME/.zshrc" ;;
      bash) rc="$HOME/.bashrc" ;;
      *) rc="" ;;
    esac
    if [ -z "$rc" ]; then
      info "Add $bindir to your PATH manually."
      return
    fi
    line="export PATH=\"$bindir:\$PATH\""
    if [ -f "$rc" ] && grep -Fq "$line" "$rc"; then
      ok "$rc already adds $bindir to PATH"
      return
    fi
    printf '\n# Added by the Malvado installer\n%s\n' "$line" >> "$rc"
    ok "Added $bindir to PATH in $rc (restart your shell or run: source $rc)"
    return
  fi
  say ""
  info "The CLI is not on your PATH yet. Add it with:"
  info "  export PATH=\"$bindir:\$PATH\""
  info "To keep it, add that line to your shell rc file, or rerun with --add-to-path."
}

main() {
  parse_args "$@"
  require_tools
  if [ "$UNINSTALL" = "1" ]; then
    do_uninstall
    return
  fi
  detect_platform
  resolve_version

  archive="${PROJECT}-sdk-${VERSION}-${PLATFORM}.tar.gz"
  base="${DOWNLOAD_BASE}/${TAG}"
  say "Installing Malvado ${VERSION} for ${PLATFORM}"

  tmp="$(mktemp -d)"
  trap 'rm -rf "$tmp"' EXIT
  say "Downloading ${archive}"
  download "${base}/${archive}" "$tmp/${archive}" || die "download failed: ${base}/${archive}"
  download "${base}/${archive}.sha256" "$tmp/${archive}.sha256" || die "checksum download failed"

  expected="$(awk 'NR==1{print $1}' "$tmp/${archive}.sha256" | tr 'A-F' 'a-f')"
  [ -n "$expected" ] || die "the checksum file is empty or malformed"
  actual="$(sha256_of "$tmp/${archive}" | tr 'A-F' 'a-f')"
  if [ "$expected" != "$actual" ]; then
    die "checksum mismatch for ${archive}
  expected: ${expected}
  actual:   ${actual}
The download is corrupt or was tampered with. Nothing was installed."
  fi
  ok "Checksum verified"

  say "Extracting"
  mkdir -p "$tmp/extract"
  if tar tzf "$tmp/${archive}" | grep -E '(^/|(^|/)\.\./)' >/dev/null 2>&1; then
    die "the archive contains unsafe paths"
  fi
  tar -xzf "$tmp/${archive}" -C "$tmp/extract" || die "extraction failed"
  [ -x "$tmp/extract/bin/${PROJECT}" ] || [ -f "$tmp/extract/bin/${PROJECT}" ] || die "the archive does not contain bin/${PROJECT}"
  if [ -f "$tmp/extract/share/malvado/malvado-sdk-info.txt" ]; then
    sdk_version="$(sed -n 's/^version=//p' "$tmp/extract/share/malvado/malvado-sdk-info.txt")"
    [ -z "$sdk_version" ] || [ "$sdk_version" = "$VERSION" ] || die "archive version ${sdk_version} does not match ${VERSION}"
  fi

  install_prefix "$tmp/extract"

  prefix="$(expand_prefix "$(default_prefix)")"
  say "Verifying"
  installed="$("$prefix/bin/${PROJECT}" --version 2>/dev/null || true)"
  [ -n "$installed" ] || die "the CLI did not report a version"
  ok "$installed"
  if "$prefix/bin/${PROJECT}" doctor >/dev/null 2>&1; then
    ok "malvado doctor passed"
  else
    info "malvado doctor reported an optional capability as unavailable"
  fi

  maybe_add_to_path "$prefix/bin"
  say "Done. Start a game with:"
  info "  malvado new my-game && cd my-game && malvado run"
}

main "$@"
