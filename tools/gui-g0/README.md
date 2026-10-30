# G0 GUI prototypes

These prototypes compare raygui and Dear ImGui for the optional Malvado GUI. Each shows a sample process tree, a generated atlas region, and the window DPI scale. The data is fixed; none is connected to the engine or an editor. Use them to inspect focus, text, resizing, and scale on each platform.

From the repository root, build and run it:

```sh
mise run build-g0
./build-g0/gui-g0-raygui
./build-g0/gui-g0-imgui
./build-g0/gui-g0-imgui-raylib
```

The first build downloads pinned raylib, Lua, cJSON, raygui 4.0, Dear ImGui 1.92.7, and an rlImGui revision for raylib 5.5. Run `mise run test-g0` for short smoke tests; they need a display or virtual display. The tests check that the windows open and close. They do not verify DPI, focus, or visual quality. If an earlier G0 build fails to link after a dependency update, run `cmake --build build-g0 --target clean` and build again.

The raygui prototype uses C and raylib. It composes the sample tree from buttons. The Dear ImGui prototypes use C++ and built-in tree and table widgets. One uses GLFW and OpenGL directly; the other uses the rlImGui adapter and raylib. They do not test docking. None is linked into the game runtime. An interactive comparison of focus, DPI, and usability on macOS and Linux remains pending. See the [GUI plan](../../docs/specs/gui.md).
