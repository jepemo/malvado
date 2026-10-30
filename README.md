# Malvado

Malvado is a game engine inspired by DIV Games Studio. The new runtime is written in C, uses raylib, and includes Lua. You can create games in Lua or use the C API directly.

> Malvado 2 is under development. The API and tools can still change.

## Try the Lua example

Install [mise](https://mise.jdx.dev/getting-started.html) and a C compiler. On Linux, you also need the X11 and OpenGL development packages listed in [CI](.github/workflows/ci.yml). The first setup needs network access. CMake downloads the pinned Lua, raylib, and cJSON sources and checks their hashes.

```sh
mise install
mise run test
mise run hello
```

Close the window or press Escape to exit. Run `mise tasks` to see all commands. `mise run test-c` tests the C core without downloading Lua or raylib. `mise run test-headless` tests Lua without a graphics backend. `mise run test-visual` needs a display or virtual display.

CMake also works without mise. To use local source trees instead of downloads, pass `-DFETCHCONTENT_SOURCE_DIR_LUA=/path/to/lua`, `-DFETCHCONTENT_SOURCE_DIR_RAYLIB=/path/to/raylib`, and `-DFETCHCONTENT_SOURCE_DIR_CJSON=/path/to/cjson` when you configure CMake. See the [runtime guide](docs/versions/2.0.0-alpha.1.md) for build options and the [C process example](examples/c/processes/README.md) for a build without Lua or raylib.

## Examples and documentation

Try the migrated [Hello World](examples/helloworld/README.md) example with Malvado 2. The `demo`, `space_asteroids`, and `bench` examples still use the old LÖVE engine. To run their original versions, use the [last pre-migration source revision](https://github.com/jepemo/malvado/tree/4670d8a). Their migration is planned for [alpha.3](docs/versions/2.0.0-alpha.3.md).

Explore the [documentation index](docs/README.md), [version checklist](docs/versions/README.md), and [migration roadmap](docs/planning/roadmap.md) for current status and plans.

## License

The Malvado 2 rewrite uses the [Apache License 2.0](LICENSE). Dependencies retain their own licenses. See [licenses and attribution](docs/development/licensing.md) for historical code and asset provenance.
