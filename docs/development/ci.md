# CI, documentation, and versions with GitHub Actions

Status: initial workflow implemented and run on GitHub on 2026-10-29. The goal is to detect failures in proposed changes, produce verifiable binaries, and publish documentation and versions when ready. This phase does not create or publish a release.

## What runs and when

| Proposed workflow | When | Work | Result |
| --- | --- | --- | --- |
| `ci.yml` | PR, push to main branch, manual | Build, CTest, code and documentation checks | Visible checks; logs and diagnostic artifacts on failure |
| `docs.yml` | PR, push to main branch, manual | Regenerate catalog, check links, build site | PR validation; main can publish Pages after repository configuration |
| `release.yml` | Manual candidate preparation; `v*` tag after process validation | Build, test, package, hash, prepare release | Platform artifacts and draft release for review |
| `nightly.yml` | Manual or scheduled when useful | Sanitizers, slow tests, more combinations | Additional diagnostics; does not replace PR CI |

Publishing requires GitHub permissions and Pages configuration. Writing workflows alone does not configure or publish a site. External PRs run without signing or deployment secrets. Use stable check names for branch protection where available.

`.github/workflows/ci.yml` has a macOS/Linux matrix, catalog verification, sanitizers and static analysis, and a Linux Xvfb graphics test. All seven earlier jobs passed in [GitHub Actions for commit 09215e8](https://github.com/jepemo/malvado/actions/runs/40908427079). Windows is validated in alpha.6. The link does not validate later changes.

## Initial CI: alpha.1

1. **Linux and macOS:** configure CMake with documented versions/toolchains, build, and run `ctest --output-on-failure`. Install raylib system dependencies. Add and validate Windows in alpha.6; it does not block earlier alphas.
2. **Raylib/Lua:** CMake downloads Lua 5.4.8 and raylib 5.5 into the build directory and verifies SHA-256 hashes. Cache downloads where useful without skipping verification. Limit style checks to project code; exclude `vendor/` and `_deps/`.
3. **Static checks and style:** enable compiler warnings for `src/`. Add `clang-format --dry-run --Werror` when configuration is applied, then `clang-tidy` on `compile_commands.json` with agreed rules. Add `luacheck` for Lua examples and tests when configured. Document known warnings and exceptions. Do not describe an unconfigured tool as an active check.
4. **Memory tests:** use ASan/UBSan in a compatible Linux configuration as process tests expand. Record platform limits. Add creation/removal, error, and coroutine cleanup cases.
5. **Documentation:** download pinned sources with `python3 docs/tools/fetch_api_sources.py`, run `python3 docs/tools/catalog_api.py`, compare generated files with the repository, and fail on differences. Both commands check source hashes. Validate internal links and site build when a generator exists. Also check generated files if they are initially untracked.
6. **Graphics example:** headless tests do not prove that a window opens. Add bounded graphics smoke tests on suitable runners, such as Linux with a virtual display, and tests on real systems. Record macOS and audio/input acceptance separately when automation is unreliable. Validate Android and Windows in alpha.6.

Recommended initial matrix: Linux x86_64 and macOS arm64; add Windows x86_64 in alpha.6. Use explicit runner/architecture names and pin versions during implementation. Confirm runner availability for this repository. If an architecture is unavailable, leave its check pending or use an appropriate runner; do not label the target validated. Do not run interactive hello indefinitely in CI; add an automatic-exit smoke mode.

## Check quality

- Each job fails on an incorrect build, test, or regeneration. Do not hide required failures with `continue-on-error`. Add timeouts to tests and jobs to detect Lua processes that do not yield.
- Build and test cleanly. Do not commit build directories or caches. Retain CI diagnostic logs and test results as temporary artifacts with bounded retention.
- Key caches by OS, architecture, compiler, dependency versions, and hashes. A cache is not proof of reproducibility and does not replace periodic clean builds.
- Apply format and analysis to project code. Pinned external files retain their format and licenses. Document tool versions and rules, and update them through PRs.
- Add example, asset, and GUI checks when implemented. For example, generate a CLI project and run its smoke test when a template exists.

## Documentation for both APIs

The [documentation plan](api-documentation.md) requires each PR that changes a public API to update the corresponding C/Lua signatures and records and run related examples. The documentation job will build MkDocs in strict mode and check links and names between the binding and Lua definitions. Implement and pin example extraction and signature checkers before claiming they exist. A site build does not replace example tests.

## Documentation and GitHub Pages

Markdown in `docs/` remains the source. As proposed in [publishing](publishing.md), pin MkDocs and initially use its default theme. PRs regenerate the catalog and build documentation without deploying. After Pages configuration, pushes to the main branch deploy development documentation. When versioning exists, keep stable documentation from tags under `/2.0/` and development under `/dev/`. Define retention before deleting published versions.

[GitHub documents](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages) Actions builds and deployments. A Pages job needs `pages: write` and `id-token: write`; grant these only to the deployment job, not PRs. Add Lua/C reference generation when Malvado contracts and tools exist. Do not automatically turn Bennu symbols into the Malvado API.

## Version generation

A release must represent an exact revision and pass CI. Initially, generate candidate artifacts with a manual workflow, review installation, then create a **draft release**. Once reliable, allow a tag such as `v2.0.0-alpha.1` to trigger the same pipeline. Alpha and beta are prereleases. Draft publication, production signing, and store uploads follow the agreed release process.

| Delivery | First planned artifact | Check before claiming support |
| --- | --- | --- |
| Desktop runtime | Executable/package for Linux x86_64, Windows x86_64, macOS arm64 | CTest, example start, included dependencies, clean-machine test |
| SDK/CLI | Tool and runtimes for available targets | New project → run → export → run package |
| Android | Debug APK for tests; signed APK/AAB for release when exporter exists | Install, test lifecycle/touch, validate AAB; signing configuration separate |
| GUI | Desktop packages with independent version | Project, preview, execution, and export flow; declared runtime compatibility |

Do not create binaries for targets not implemented. A runner alpha binary is not a game exported by `malvado-cli`. For each candidate, record tag/commit, version, platform/architecture, toolchain, dependencies, licenses, SHA-256 checksum, and change notes. The changelog contains delivered changes only. Verify tag and metadata versions before packaging. Do not overwrite an existing release artifact.

macOS distribution may require signing and notarization according to channel. Android signed variants need keys. Keep credentials in secrets/protected environments for authorized release jobs only; ordinary CI does not need keys. First distribute test artifacts without claiming validated installation. Do not promise identical signed binaries. Record inputs and compare reproducible outputs where possible.

## Verify C core and Lua binding

Alpha.2 requires a clean job with Lua and raylib disabled: build and install the C library with the headless backend, compile a C consumer against installed headers, and run it. Confirm that Lua/raylib are not downloaded or linked. Also build the official runner/binding, run existing Lua tests and C/Lua parity and mixed tests, and run sanitizers. Validate C/Lua raylib graphics smoke tests by platform. See [C architecture](../architecture/c-first.md).

## Distribution and LuaRocks

The [distribution plan](distribution.md) defines the SDK, CLI, GUI, and optional rock. A 2.0 rock needs a C module loadable by external Lua, CMake installation, and ABI tests. The bundled-Lua runner is not that module. Do not add a rock publication job until it exists. Then run lint, installation, and clean-tree tests and review licenses before upload.

## Actions permissions and dependencies

For PR CI, use `contents: read` or less and no secrets. Do not run untrusted PR code with write permissions. Publication jobs need separate minimum permissions: `contents: write` for draft releases and Pages permissions only for deployment. Pin external actions to full upstream commit SHAs and review them regularly, as [GitHub recommends](https://docs.github.com/en/actions/reference/security/secure-use). Do not put credentials in logs, artifacts, or examples.

## Adoption order

| Time | Implement | Acceptance gate |
| --- | --- | --- |
| Current alpha.1 | `ci.yml`: available desktop builds, headless CTest, timeouts, catalog, compiler warnings | Test PR shows verifiable checks on available runners; document gaps |
| Alpha.2 | Agreed formatting/analysis; sanitizers and expanded scheduler tests | CI fails on real errors and provides useful logs |
| Alpha.3 | Bounded graphics smoke tests and asset examples; site build | CI examples exit automatically; no broken internal documentation links |
| Alpha.4 | macOS/Linux packages, draft candidate, end-to-end CLI tests | Artifacts installed and run on clean hosts; correct checksums |
| Alpha.6 | Android build and APK/AAB validation; add Windows CI and validate runtime, C/Lua, distributed GUI, CLI, packages | Android emulator/device and lifecycle; Windows CI, graphics test, and clean-machine execution; release signing separate |
| Beta/2.0 | Versioned Pages, tag automation, publication criteria | Reviewed complete candidate; docs and artifacts match the same commit |
| GUI | Desktop build and smoke jobs at G0/G1 | CI covers selected toolkit and separates GUI from runtime |

CI helps decide whether a version is ready. It does not replace license review, distribution tests, or graphics experience tests on real machines.
