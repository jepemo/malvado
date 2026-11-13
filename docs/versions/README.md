# Version checklist

This page shows the release plan at a glance. Open a version to see its checklist, acceptance criteria, and related design documents. A checked item records verified work; it does not by itself mean a release package exists.

Alphas include implementation and automated tests. Automated graphics, input, clean-runner, and emulator tests remain in the relevant alpha. Manual testing of complete user flows, physical devices, audio, and clean machines is planned for beta. Do not mark unfinished features as complete when moving manual tests.

## How to plan a version

A version has a named outcome, a fixed scope, and a checklist of deliverables. Each unchecked item must name the behavior or decision, the boundary, and the automated evidence needed to close it. A checked item means that its complete scope and checks passed. Keep implementation, graphics, and emulator checks in the relevant alpha. Schedule manual user flows and physical-device checks for beta.

Do not add a checked item for each partial change. Keep partial results in a linked design or evidence page, or in commit history. If implementation reveals a gap, add a specific unchecked item to the responsible version. Moving unfinished work to another version requires an explicit scope decision and a clear destination; it does not count as completed work.

Some later versions depend on decisions that do not exist yet. Their lists are planning gates, not final implementation checklists. Resolve those gates and split the resulting work into bounded deliverables before starting that implementation. Do not mark a whole family complete without its code, contract, and automated checks.

- [x] [Alpha.1](2.0.0-alpha.1.md) — **complete**: C runtime, bundled Lua, raylib, and hello. Source tag: `v2.0.0-alpha.1`; no release package yet.
- [x] [Alpha.2](2.0.0-alpha.2.md) — **complete**: shared C/Lua scheduler, processes, signals, messages, and inspection. Source tag: `v2.0.0-alpha.2`; no release package yet.
- [x] [Alpha.3](2.0.0-alpha.3.md) — **complete**: graphics and input. Source tag: `v2.0.0-alpha.3`.
- [x] [Alpha.4](2.0.0-alpha.4.md) — **complete**: minimal client, SDK, project creation, execution, and desktop packages. Source tag: `v2.0.0-alpha.4`; no release package yet.
- [x] [Alpha.5](2.0.0-alpha.5.md) — **complete**: process model, C and Lua API, lifecycle, resources, and genealogy. The source revision selected for the tag must pass CI; no tag or release package exists yet.
- [x] [Alpha.6](2.0.0-alpha.6.md) — **complete**: simple Lua 2D APIs, unified assets, automatic animation, contextual world calls, and cameras. Candidate `d48729f` passed all required CI jobs; no source tag or published release.
- [x] [Alpha.7](2.0.0-alpha.7.md) — **complete**: native C/Lua/Rust integration, installed Rust CLI/player exports, executable guide checks and verified SDK candidates. Published prerelease: [`v2.0.0-alpha.7`](https://github.com/jepemo/malvado/releases/tag/v2.0.0-alpha.7), from `6c56838` with all 15 CI jobs passing.
- [ ] [Alpha.8](2.0.0-alpha.8.md) — **in progress**: Windows/Android support and platform/CLI/package automation for the existing engine.
- [ ] [Beta.1](2.0.0-beta.1.md) — **planned**: candidate contracts, documentation, minimal aligned examples, manual device/user-flow checks and release preparation.
- [ ] [2.0.0](2.0.0.md) — **planned**: distribute the existing engine with completed C/Lua/Rust integration and verified platform paths.

## Scope decision

The first 2.0 release validates the existing capabilities. New API families,
graphics modes, GUI, complete catalog/minigames and site redesign move to the
[future roadmap](../roadmap/README.md), without assigned versions. Historical
reference calls are candidates, not release requirements.

The former alpha.9 and alpha.10 pages remain relocation records, not unfinished
releases that block beta. Windows/Android work moves to alpha.8; release-facing
documentation moves to beta. Reassigned work is not marked complete.

Breaking changes within 2.x remain possible until the owner explicitly requests
a compatibility freeze. Beta reviews one candidate's contracts; it does not
establish permanent compatibility. Each approved API change still needs a reason,
updated tests/docs, migration guidance and correct native ABI/SDK rejection.

The existing [family review](../api/family-review.md) bounds current functionality.
iOS and exploratory APIs remain roadmap research. The older alpha renumberings
remain in Git history; preserve or redirect published URLs during future site work.

## Rust support policy

Malvado 2.0 supports C, Lua and experimental Rust. The Rust gameplay SDK is
functional, maintained and tested over the shared C scheduler. Its public API
can change in future 2.x releases, with migration guidance for significant
changes and no long-term stability guarantee. This label does not apply to the
C core. `Game` replaces the public Rust `Engine` name before 2.0; acceptance is
tracked in [beta.1](2.0.0-beta.1.md), without reopening completed alphas.
The current owned async API remains the baseline. [Future Rust ergonomics](../roadmap/rust-ergonomics.md)
are proposals without an assigned version and are not 2.0 requirements.
