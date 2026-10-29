# Malvado

Malvado is a game engine inspired by DIV Games Studio. The new runtime is written in C, uses raylib, and includes Lua. You can create games in Lua or use the C API directly.

> Malvado 2 is under development. The API and tools can still change.

## Try the Lua example

You need CMake 3.20 or later, a C compiler, and network access for the first build. CMake downloads the pinned Lua and raylib sources and checks their hashes.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/malvado-run examples/alpha-hello/main.lua
```

Close the window or press Escape to exit. See the [runtime guide](docs/versions/2.0.0-alpha.1.md) for build options and the [C process example](examples/c/processes/README.md) for a build without Lua or raylib.

## Examples and documentation

The examples in `examples/helloworld`, `examples/demo`, `examples/space_asteroids`, and `examples/bench` are kept as migration references. They use the old LÖVE engine and do not yet run on Malvado 2. To run the original versions, use the [last pre-migration source revision](https://github.com/jepemo/malvado/tree/4670d8a). Their migration is planned for [alpha.3](docs/versions/2.0.0-alpha.3.md).

Explore the [documentation index](docs/README.md), [version checklist](docs/versions/README.md), and [migration roadmap](docs/planning/roadmap.md) for current status and plans.

## License

The Malvado 2 rewrite uses the [Apache License 2.0](LICENSE). Dependencies retain their own licenses. See [licenses and attribution](docs/development/licensing.md) for historical code and asset provenance.
