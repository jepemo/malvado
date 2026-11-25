# Rust gameplay ergonomics and process declarations

## Delivered baseline

The experimental Rust SDK supports named async processes, default spawning,
direct self operations, process timing and a managed window loop. See
[Rust processes](../api/rust-processes.md) and the
[game migration guide](../development/rust-game-migration.md). The
[C scheduler](../api/c-contract.md) still owns activation, process identity and
safe-point cancellation. The [version plan](../versions/README.md) defines
release scope.

## Evaluate richer declarations

| Form | What it offers | Costs and limits |
| --- | --- | --- |
| Named async function | Sequential behavior, locals across awaits, direct Rust Analyzer navigation and ordinary compiler diagnostics. Capture parameters in a closure. | A function item has no type name usable as a generic argument such as `collision::<Enemy>()`. |
| Struct with `async fn run(self, p: ProcessContext)` | A named state type with ordinary Rust fields and methods. The spawned future owns the struct. | Spawning uses a closure such as `game.spawn(move \|p\| enemy.run(p))`. Use an owned receiver so the future needs no external borrow of the state. |
| Trait adapter for structs | An explicit typed entry point could share validation and scheduler behavior with functions. | A common spawn trait changes inference and diagnostics. It would need an explicit implementation per struct or a separate adapter. |
| Optional attribute macro | A macro could generate a marker and help register type-specific parameters and queries. | It adds expansion, generated names, error spans and Rust Analyzer navigation to check. Typed registration also needs a native identity contract. |

A bounded Rust 2024 compiler probe accepted named async functions and a
concrete owned `Enemy` implementation under a shared trait. It rejected
overlapping blanket implementations for both `FnOnce` factories and
`T: OwnedProcess` with E0119. A factory closure calling a context method
through that shared trait needed an explicit context type (E0282). The
current direct `FnOnce(ProcessContext)` bound infers it. A separate
`spawn_owned` adapter with an owned async receiver compiled; it needs no
self-reference or lifetime extension.

## Recommendation

Keep named functions and owned struct methods on the existing spawn path.
Consider a separately named owned adapter only if real examples repeat
substantial setup. Leave attribute macros and typed queries for a separately
reviewed design. Test expansion, diagnostics, and editor behavior if there is
a concrete macro proposal.

A typed query would need a real public type or marker and a mapping to the
canonical native `type_id`. The design must define ID allocation and
registration scope, collisions between IDs and manual IDs, and how type
changes affect later queries. Query results must be checked handles so an ended
process or closed game receives the normal native errors. A marker must never
lend references into a suspended future. Any adapter must use the same native
ownership, cancellation and panic boundary.

Process-associated collision worlds and a multiple-frame wait are separate
ideas. A world API needs rules for selection, foreign handles, and resource
lifetime. Extra wait syntax needs a use case and a clear definition of how
cadence and sleep affect it. Neither is part of the declaration spike.
