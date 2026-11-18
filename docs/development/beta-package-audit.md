# Beta.1 package audit baseline

The [alpha.8 candidate run](https://github.com/jepemo/malvado/actions/runs/44280840065)
provided three SDK archives from `2a20b7ae9b8cc03a21a92bd2aee29e185789f6f1`.
The [beta.1 checklist](../versions/2.0.0-beta.1.md) tracks the remaining
dependency notice, asset provenance and final package review.

## SDK archives

Download the `platform-candidate-<commit>` artifact and inspect each archive
with `tools/sdk_artifact.py` (the `inspect_archive` function). Compare its
result with the matching entry under `sdks` in `platform-candidate.json`.
Verify the archive bytes and the accompanying `.sha256` file before inspecting
the installed headers, libraries, CLI, launchers and crate sources.

| SDK | Verified SHA-256 |
| --- | --- |
| macOS arm64 | `3d42a66cb4a3b2934286dec990de703e941f7c5183ee722383510ebba93173dd` |
| Linux x86_64 | `b2bdbc04a73be0be422870d0c5911bb7a3f51f3ce189ca8994e7556d88f202f9` |
| Windows x64 | `849fffbbb4a269712f68cec7cd9e6301c31974218e2dece24a9aee96be16eb14` |

The public C headers, native libraries, both installed player launchers and
both Rust crate manifests are present on each target. The project, Lua 5.4.8,
cJSON 1.7.19 and raylib 6.2 notices match the repository and pinned build
sources. The Windows project license uses CRLF line endings. The other
notices match byte for byte.

The Windows workflow ran the exported C, Lua and Rust players on a fresh
runner with the SDK and authoring tools off its PATH. Linux clean-player
verification uses a separate runtime container. The
[platform matrix](alpha8-platforms.md) lists the OS graphics and runtime
requirements for players.

## Source archive

The [source archive verifier](distribution.md#source-archive) passed on
macOS arm64 at `d64e05238949a629f75d911f697cee35a4838639`. Its SHA-256 was
`8525e45c67bf5103b2bb2d41c041b5c8f5a69a47659a225b9a5ca2f7e8ac4ec5`.
The extracted core passed all 99 tests. The full desktop SDK built and
installed, the external C SDK test passed, and the four installed notices
matched their source files.

## Android export evidence

The [alpha.8 Android run](https://github.com/jepemo/malvado/actions/runs/44277183559)
provides separate Debug and Release `android-production-<commit>-<profile>`
artifacts. The C, Lua and Rust result files record 24 successful launches
with cleanup across Debug APKs, Release APKs and APKs derived from Release
AABs. This is player execution on the emulator. The downloaded evidence
contains reports and logs. Review the final APK and AAB bytes, notices and
checksums when assembling the beta.1 candidate.

## Open review items

- Identify the notices for the versions of the libraries that raylib 6.2
  compiles into the SDK and players. The SDK notice directory includes the
  standalone raylib license. The pinned build also contains dependencies
  with their own notice text. For example, the macOS archive exports
  `qoi_decode` and the pinned `qoi.h` carries a separate MIT notice.
  Check the desktop and Android outputs against the actual linked code.
  Use the pinned source and the
  [raylib dependency inventory](https://github.com/raysan5/raylib/wiki/raylib-dependencies).
- Confirm and record provenance for the historical demo and Space Asteroids
  images included in the source archive. The font fixtures already have
  [license records](../../tests/fixtures/fonts/README.md) and the Coric
  fixture has [attribution](../../tests/fixtures/aseprite/coric/README.md).
- Repeat package inspection and player checks on the selected beta.1
  revision. Retain the final artifact checksums and evidence together.
