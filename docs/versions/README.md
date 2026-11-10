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
- [ ] [Alpha.7](2.0.0-alpha.7.md) — **planned**: multi-language architecture and first-class Rust.
- [ ] [Alpha.8](2.0.0-alpha.8.md) — **planned**: complete the agreed 2.0 API in C, Lua, and Rust.
- [ ] [Alpha.9](2.0.0-alpha.9.md) — **planned**: Android and Windows.
- [ ] [Alpha.10](2.0.0-alpha.10.md) — **planned**: complete the public site and verify distribution.
- [ ] [Beta.1](2.0.0-beta.1.md) — **planned**: freeze the complete 2.0 API and verify release candidates.
- [ ] [2.0.0](2.0.0.md) — **planned**: full agreed API, client, documentation, and verified macOS, Linux, Windows, and Android packages.
- [ ] **2.1** — [Mode 7](../specs/graphics-modes.md).
- [ ] **2.2** — [Mode 8](../specs/graphics-modes.md).
- [ ] **2.3** — [Mode 9](../specs/graphics-modes.md).
- [ ] **2.4** — [GUI](../specs/gui.md).
- [ ] **2.5** — review additional Aseprite metadata and export formats after the 2.0 API and GUI; see the [asset decision](../specs/assets.md#alpha6-aseprite-acceptance-decision).
- [ ] **2.x, later** — evaluate LuaJIT using Malvado workloads and compatibility tests; Lua 5.4 remains the 2.0 runtime.

The GUI and graphics modes are outside the 2.0 release gates. iOS is deferred to a later version. The full 2.0 API inventory needs an explicit decision; historical compatibility is tracked separately. See the [roadmap](../planning/roadmap.md) and [CLI plan](../specs/cli.md).

The planned alpha.7–alpha.9 milestones were renumbered when Rust was inserted as alpha.7. Alpha.8 and alpha.9 were subsequently exchanged. Old links to those version filenames may now identify a different milestone; the alpha.10 website migration must audit public links and explain this change.
