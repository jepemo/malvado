# G0 raygui prototype

This prototype is a small test for the optional Malvado GUI. It shows a sample process tree, a generated atlas region, and the window DPI scale. The data is fixed; it is not connected to the engine or an editor. Use it to inspect focus, text, resizing, and scale on each platform.

From the repository root, build and run it:

```sh
mise run build-g0
./build-g0/gui-g0-raygui
```

The first build downloads pinned raylib, Lua, cJSON, and raygui sources. The prototype uses raygui 4.0. Run `mise run test-g0` for a short smoke test; it needs a display or virtual display. The test checks that the window opens and closes. It does not verify DPI, focus, or visual quality.

This is one candidate in the [GUI comparison](../../docs/specs/gui.md). An ImGui prototype and an interactive comparison on macOS and Linux remain pending. The prototype is not linked into the game runtime.
