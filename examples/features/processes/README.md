# Processes

Run two workers at the same priority. Each takes one turn and yields before
its second turn. The scheduler gives both workers a turn on the first tick
and resumes them in the same order on the next tick.

[C](c/main.c) · [Lua](lua/main.lua) · [Rust](rust/main.rs)

Use the [shared SDK setup](../README.md#prepare-a-project) with
`examples/features/processes` as the lesson source. Enter the generated
`lesson` directory and run `malvado run` for C or Rust, or
`malvado run --headless` for Lua.

The workers finish after this trace:

```text
first: 1
second: 1
first: 2
second: 2
```

There are no controls or assets. The sources use the project
[Apache 2.0 license](../../../LICENSE).
