# Continuous integration and release candidates

| Workflow | Coverage |
| --- | --- |
| `ci.yml` | macOS and Linux builds, tests, sanitizers, API inventories, documentation, source archive and installed SDK consumers. |
| `windows.yml` | Windows x64 MSVC SDKs, graphics, CLI and exported players. |
| `android.yml` | API 34 emulator fixtures and production APK and AAB export checks. |
| `platform-candidate.yml` | Exact-revision platform gate, SDK collection and optional draft release bundle. |
| `pages.yml` | Builds and publishes the documentation site. |

Pull requests run the standard build and tests, installed Rust SDK tests,
API catalog checks and documentation checks on Ubuntu. The documentation
job checks reference coverage, guide links, the built site with published
snapshots, and navigation and search in Chromium. It checks the current
site downloads with an installed SDK and a software-rendered X11 session.
See the [website verification commands](../website/README.md#run-the-documentation-checks)
and [desktop example requirements](../website/README.md#run-the-published-examples).
Each push to `main`, including a merge, runs the full Linux, macOS, Windows
and Android checks. You can also run each full workflow manually on a branch.
Windows and Android also run on their weekly schedules.

Run bounded tests and retain failures as diagnostic artifacts. Record hardware
and audible checks separately from emulator and software-rendered results.
Use the [platform matrix](platforms.md) for supported paths and
[development commands](../../mise.toml) for local tasks.

Select one revision for release. Verify the SDKs, source archive, notices,
checksums and versioned documentation from that revision. Keep signing and
publication separate from normal CI. See [distribution](distribution.md)
and [publishing](publishing.md).

## Rust sanitizer limits

`mise run test-rust-graphics-sanitize` runs Rust tests against instrumented
native libraries. Leak detection is disabled on macOS. On Linux, the
graphics check filters known driver allocation stacks. It still checks
Malvado allocations and verifies that unrelated leaks are reported.

## Android foundation job

`android.yml` uses an isolated API 34 x86_64 emulator on Ubuntu 24.04. It runs
`mise run test-android` in Debug and Release with the pinned NDK r27c. The same
script verifies native C/Lua executables, a relocated installed C SDK consumer,
Lua errors/cleanup and shared C/Lua traces. It uploads bounded command logs and
actual tool revisions on failure. The Rust headless job checks installed
SDK consumers and ABI rejection. Graphical C, Lua and Rust fixtures verify
startup, framebuffer pixels, input, lifecycle and cleanup. See
[Android verification](android.md) for the local commands and evidence.

## Android production packages

The Android workflow also builds a fresh installed CLI and arm64 target SDKs
on Ubuntu 24.04. Debug verifies C, both Lua entry forms and Rust as APKs. Release
verifies APKs and AABs, including APKs derived from the bundles. The job runs
the arm64 packages through native translation on the pinned x86_64 emulator.
It requires `arm64-v8a` support for apps and translated executables for the
Rust ABI probes. It retains runtime logs,
screenshots, manifest dumps and tool provenance. A second test APK verifies
transient and permanent Android audio-focus loss in the exported C game. The
IME fixture verifies keyboard visibility and committed Unicode across C, both
Lua entry forms and Rust. It restores the emulator's previous input method. Signing uses disposable
keys.

Run the same command on a configured authoring host with the pinned SDK:

```sh
mise exec -- bash tools/verify-android-exports.sh "$ANDROID_SDK_ROOT" Debug build-android/production-debug
mise exec -- bash tools/verify-android-exports.sh "$ANDROID_SDK_ROOT" Release build-android/production-release
```

Each command needs a new work directory. Add an explicit API 34 emulator serial
as the last argument to verify installation, runtime behavior and uninstall.
Emulator execution also verifies Rust and requires the `aarch64-linux-android`
Rust target for the executable ABI probes. With no
serial, the command verifies C and Lua package contents on the authoring host.

The candidate gate requires branch runs.

## Supported-platform candidate gate

Run the manual **Platform candidate** workflow after CI, Windows and Android
foundation finish on the same commit. Supply the full commit SHA and the three
run IDs. The workflow checks repository, workflow path, branch event, revision,
run conclusion and every job in the selected run attempt. Missing, skipped or
unfinished jobs fail the gate. It collects desktop SDK archives and uploads a JSON report only after all
three platform workflows pass. A rerun requires a new gate execution.

Use the same validator locally with authenticated GitHub CLI access:

```sh
python3 tools/platform_candidate.py --sha FULL_COMMIT_SHA \
  --ci CI_RUN_ID --windows WINDOWS_RUN_ID --android ANDROID_RUN_ID \
  --output platform-candidate.json
```

After the gate passes, the workflow downloads the macOS/Linux SDK candidate
from the selected CI run and the Windows SDK from the selected Windows run.
The collector requires exactly macOS arm64, Linux x86_64 and Windows AMD64,
one common SDK version, valid checksums and matching archive metadata. It
uploads all three archives, checksums and a report with source run IDs and
archive digests. Expired or missing artifacts fail collection. Existing output
folders are preserved; failed copies remove only the new output folder.

Run the same collection locally after downloading those artifacts:

```sh
python3 tools/collect_platform_sdk.py --desktop incoming/desktop \
  --windows incoming/windows --report platform-candidate.json \
  --output platform-candidate
```

This report records platform test evidence. Release publication and review
of notices and artifacts happen separately. The CI-only SDK candidate
contains macOS and Linux packages. The Platform candidate adds Windows.

### Draft release bundle

Pass the optional version to **Platform candidate** to assemble a draft
release bundle. Use the same complete CI, Windows and Android runs required
by the platform gate. The workflow downloads the source archive from the
selected CI run, builds versioned documentation from the selected commit
and previews the [canonical release notes](publishing.md#finalize-release-notes)
for that version. The collector renders the GitHub body from the same source
archive and checks its digest against the candidate documentation metadata.
The bundle requires nonempty, matching notes and valid repository links.

The `release-candidate-<version>-<commit>` artifact includes the three
desktop SDKs, the source archive, the reports, the versioned site and
`RELEASE-NOTES.md`. The collector checks the source digest, exact SDK versions
and dependency notices against the source archive. It rejects a failed source
verification or documentation for another commit. From the extracted bundle
directory, check every file with `sha256sum -c SHA256SUMS` on Linux or
`shasum -a 256 -c SHA256SUMS` on macOS.

Use the Android run linked in `platform-candidate.json` for emulator
evidence. The draft bundle is a CI artifact for review. Manual tests,
production signing and publication have their own acceptance steps.
