# Control C processes

This headless example creates an owned child and an independent child. An
ownership signal affects only the owned child. A creation-tree signal affects
both. It also changes priority and type, kills the root through a reference,
and reads the retained termination reason after context destruction.

Build the project and run `build/example-c-control`. It also works with
`-DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF`. See the
[C contract](../../../docs/api/c-contract.md) for errors and lifetime rules.
