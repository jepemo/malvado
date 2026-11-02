# Version checklist

This page shows the release plan at a glance. Open a version to see its checklist, acceptance criteria, and related design documents. A checked item records verified work; it does not by itself mean a release package exists.

- [x] [Alpha.1](2.0.0-alpha.1.md) — **complete**: C runtime, bundled Lua, raylib, and hello. Source tag: `v2.0.0-alpha.1`; no release package yet.
- [x] [Alpha.2](2.0.0-alpha.2.md) — **complete**: shared C/Lua scheduler, processes, signals, messages, and inspection. Source tag: `v2.0.0-alpha.2`; no release package yet.
- [ ] [Alpha.3](2.0.0-alpha.3.md) — **in progress**: 2D graphics, input, text, audio, examples, and development workflow. Review before tagging.
- [ ] [Alpha.4](2.0.0-alpha.4.md) — **planned**: minimal client, SDK, project creation, execution, and desktop packages.
- [ ] [Alpha.5](2.0.0-alpha.5.md) — **planned**: define the full 2.0 API inventory, collision, scrolling, and asset importers.
- [ ] [Alpha.6](2.0.0-alpha.6.md) — **planned**: Android and Windows.
- [ ] [Alpha.7](2.0.0-alpha.7.md) — **planned**: complete the agreed 2.0 API in C and Lua.
- [ ] [Alpha.8](2.0.0-alpha.8.md) — **planned**: complete the public site and verify distribution.
- [ ] [Beta.1](2.0.0-beta.1.md) — **planned**: freeze the complete 2.0 API and verify release candidates.
- [ ] [2.0.0](2.0.0.md) — **planned**: full agreed API, client, documentation, and verified macOS, Linux, Windows, and Android packages.
- [ ] **2.1** — [Mode 7](../specs/graphics-modes.md).
- [ ] **2.2** — [Mode 8](../specs/graphics-modes.md).
- [ ] **2.3** — [Mode 9](../specs/graphics-modes.md).
- [ ] **2.4** — [GUI](../specs/gui.md).

The GUI and graphics modes are outside the 2.0 release gates. iOS is deferred to a later version. The full 2.0 API inventory needs an explicit decision; historical compatibility is tracked separately. See the [roadmap](../planning/roadmap.md) and [CLI plan](../specs/cli.md).
