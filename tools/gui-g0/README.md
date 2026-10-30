# G0 GUI prototypes

These prototypes compare raygui and Dear ImGui for the optional Malvado GUI. Each shows a sample process tree, a generated atlas region, and the window DPI scale. The data is fixed; neither prototype is connected to the engine or an editor. Use them to inspect focus, text, resizing, and scale on each platform.

From the repository root, build and run it:

```sh
mise run build-g0
./build-g0/gui-g0-raygui
./build-g0/gui-g0-imgui
```

The first build downloads pinned raylib, Lua, cJSON, raygui 4.0, and Dear ImGui 1.91.9b sources. Run `mise run test-g0` for short smoke tests; they need a display or virtual display. The tests check that both windows open and close. They do not verify DPI, focus, or visual quality.

The raygui prototype uses C and raylib. It composes the sample tree from buttons. The Dear ImGui prototype uses C++, GLFW, and OpenGL; it uses built-in tree and table widgets. It does not test an ImGui-to-raylib adapter or docking. Neither prototype is linked into the game runtime. An interactive comparison of focus, DPI, and usability on macOS and Linux remains pending. See the [GUI plan](../../docs/specs/gui.md).
