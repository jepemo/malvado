# Version checklist

This page shows the release plan at a glance. Open a version to see its checklist, acceptance criteria, and related design documents. A checked item records verified work; it does not by itself mean a release package exists.

- [x] [Alpha.1](2.0.0-alpha.1.md) — **complete**: C runtime, bundled Lua, raylib, and hello. Source tag: `v2.0.0-alpha.1`; no release package yet.
- [x] [Alpha.2](2.0.0-alpha.2.md) — **complete**: shared C/Lua scheduler, processes, signals, messages, and inspection. Source tag: `v2.0.0-alpha.2`; no release package yet.
- [ ] [Alpha.3](2.0.0-alpha.3.md) — **in progress**: graphics, input, text, audio, migrated examples, mise development workflows, and GUI toolkit prototype G0.
- [ ] [Alpha.4](2.0.0-alpha.4.md) — **planned**: CLI, C SDK, desktop C/Lua packages, and the basis for GUI G1.
- [ ] [Alpha.5](2.0.0-alpha.5.md) — **planned**: essential Bennu profile, collision, and first asset importers.
- [ ] [Alpha.6](2.0.0-alpha.6.md) — **planned**: Android prototype/export and full Windows validation.
- [ ] [Beta.1](2.0.0-beta.1.md) — **planned**: freeze the essential API and complete its documentation.
- [ ] [2.0.0](2.0.0.md) — **planned**: stable essential profile and accepted exporters.
- [ ] [2.1.0-alpha.1–alpha.3](../specs/graphics-modes.md) — **proposed**: separate Mode 7, Mode 9, and Mode 8 MVPs after stable 2.0.
- [ ] [2.1.0-beta.1 and 2.1.0](../specs/graphics-modes.md) — **proposed**: stabilize the modes selected for release.
- [ ] [2.1 and later](../planning/roadmap.md) — **planned**: extended families, more FPG variants, FPG export, and mountable ZIP. These remain independently scheduled.
- [ ] [2.2 and later](../specs/graphics-modes.md) — **candidates**: advanced graphics styles and compatibility research.

## CLI and GUI

`malvado-cli` starts in alpha.4, with project creation, run/check, and macOS/Linux export. The basic asset importers are planned for alpha.5. Android export and full Windows validation are in alpha.6. See the [CLI plan](../specs/cli.md).

The optional GUI has its own version numbers. G0 is a toolkit prototype after core alpha.3. G1 (`gui 0.1.0`) depends on core and CLI alpha.4. G2 adds process inspection and tick controls; G3 adds asset metadata editing; G4 is the complete supported-host flow. GUI releases do not block the runtime or CLI. See the [GUI plan](../specs/gui.md).

## Platform and release policy

Alphas 1–5 require macOS and Linux checks. Android and Windows tests and fixes are scheduled for alpha.6. A successful CI build alone does not validate an installer or a clean-machine graphics experience. The runtime, Lua binding, and CLI initially use coordinated versions. The optional GUI declares its own compatibility. See [CI and releases](../development/ci.md) and [distribution](../development/distribution.md).

Define the essential Bennu profile before implementing its families. Do not claim full compatibility without its completed matrix. Documented extended profiles can remain deferred. See [compatibility profiles](../api/compatibility.md) and the [roadmap](../planning/roadmap.md).
