# Beta.1 package audit

The [alpha.8 candidate run](https://github.com/jepemo/malvado/actions/runs/44280840065)
provided three SDK archives from `2a20b7ae9b8cc03a21a92bd2aee29e185789f6f1`.
This baseline established the checks repeated on
[the revised beta.1 candidate](#revised-beta1-candidate).

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

## Dependency notices

The alpha.8 SDKs include the standalone raylib license. For example, the
macOS archive also exports `qoi_decode`, and the pinned `qoi.h` has a
separate MIT notice. Beta.1 adds the [raylib and NDK dependency notices](licensing.md#dependencies-and-references)
to graphical SDKs and game exports. The desktop SDK and Android package
checks require the new notice file.

On macOS arm64, the beta.1 installed SDK test verified that the desktop
export fails when the notice file is removed. Debug Android packages in C,
both Lua entry forms and Rust contained byte-identical copies and ran on
the local arm64 API 34 emulator. The Rust CLI tests verified notices in
desktop game exports.

## Example artwork

The [demo artwork record](../../examples/demo/README.md#artwork) covers the
character sheet and six cat frames. The unused bird sheet and cursor were
removed. [Space Asteroids](../../examples/space_asteroids/README.md) uses
original art for the ship and asteroid and a primitive for shots. The font
fixtures have [license records](../../tests/fixtures/fonts/README.md) and the
Coric fixture has [attribution](../../tests/fixtures/aseprite/coric/README.md).

## Revised beta.1 candidate

The package review used `d502125ece681074e9a302798c9793a68fdca603`.
The [CI run](https://github.com/jepemo/malvado/actions/runs/44320665075)
provided the Linux SDK and verified source archive. The
[Windows run](https://github.com/jepemo/malvado/actions/runs/44320664984)
provided the Windows SDK and six Debug/Release game folders. The macOS SDK
was built locally from a clean extraction of the same revision. This table
identifies the archives inspected during the audit.

| Archive | SHA-256 |
| --- | --- |
| macOS arm64 SDK, local build | `b092acf8abb28656f5bd350dfb7ee9a55616a69879af5a430aca62506d1bda64` |
| Linux x86_64 SDK | `2e1cd0a7f7c46c9094656dcf4df6e076d2a98915060e66ae8b5896daa9a94492` |
| Windows x64 SDK | `75ef58b8491a1f4f73ab48dbd663169510bed0fc9982b591de49733fa78d9bf6` |
| Source archive | `2da9a3c82fa34f7e70830022b58521bf9008deef0ecb06ca1a07bf596129b5b5` |

All three SDKs contained the CLI, both player launchers, public headers,
the native core, asset and graphics libraries, and both Rust crate
manifests. The project license and the Lua, cJSON, raylib and bundled
dependency notices matched the pinned sources, with line endings normalized.
The six Windows export folders also preserved all five notices. The source
archive included the new sprites, the demo attribution and the fixture
licenses. The retired artwork was absent. The source verifier passed the
core and external installed SDK tests.

Six Linux exports ran in a separate container with no SDK or authoring
toolchain. Windows C, Lua and Rust exports ran on a fresh runner in Debug
and Release. On macOS, all nine lessons passed from the relocated SDK and
their exported folders with authoring tools off PATH. See the
[authoring walkthroughs](beta-authoring-verification.md).

### Android Release packages

Built the image lesson in C, Lua and Rust with the installed beta.1 CLI
and the arm64 target SDK from the audited source. The six packages contained
the arm64 library, Activity bytecode, shared PNG and applicable notices.
Native project source files were excluded. All bundles passed bundletool
validation. All APKs and the three derived APKs passed signature checks and
ran to a successful exit on the API 34 arm64 emulator. The test signing
key was disposable and the packages were uninstalled after each run.

| Language | Release APK SHA-256 | Release AAB SHA-256 |
| --- | --- | --- |
| C | `b6c89d3a8b4821b74dca0ed1b07b05b734c1b5a6ed44b098a81c477afccb82f4` | `58c19dba8e7b90b2525a6566284c8078bc31b8bf7707fb038497c75829e8c31a` |
| Lua | `6cbbe7b2df01498071ef11ed6c6fc5035bc57cec385a7c61a04dd960fc7d9f84` | `53aa474bd8f7049015e1bcb6c0c2a2dab204e97b112c538f1e6e7d988fefa685` |
| Rust | `9fce9d0d0b3e6408beb765f331f6951319da1c10ce7e61bbeb56476205fec947` | `2b0f5fcc845b6d0e05f3acd54018ee82741576bf1409ededc629acaf16b10849` |

These checksums identify test outputs. The publication bundle must come
from the exact-revision platform gate. Hardware and manual user-flow
acceptance remain in the [beta.1 checklist](../versions/2.0.0-beta.1.md).
