# Startup

Initialize Malvado, print `ready` once and exit. The C and Rust versions
create a game context. Lua returns a game table and runs its `init` function
once through the bundled runner.

[C](c/main.c) · [Lua](lua/main.lua) · [Rust](rust/main.rs)

Use the [shared SDK setup](../README.md#prepare-a-project) with
`examples/features/startup` as the lesson source. Enter the generated
`lesson` directory and run `malvado run` for C or Rust, or
`malvado run --headless` for Lua.

All versions print `ready` and exit without opening a window. There are no
controls or assets. The sources use the project [Apache 2.0 license](../../../LICENSE).
