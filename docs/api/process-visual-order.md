# Process visual depth and order

Each native process owns a finite `z`, initially zero. Larger values draw later.
Equal values use creation-ID order. Scheduler priority is separate and does not
change visual depth. Native C, Lua rendering and Rust queries share this state.

## C

`mv_process_z(ctx, reference, &z)` copies a live process depth.
`mv_process_set_z(ctx, reference, z)` changes it. Non-finite values and foreign
references return `MV_INVALID_ARGUMENT`; dead processes return `MV_NOT_FOUND`.
Failure preserves state and output. Both operations need a live owner-thread
context and allocate nothing.

`mv_process_draw_order(ctx, out, capacity, &total)` copies `MvProcessDrawInfo`
records with context-local IDs and depths. It includes running, frozen and
not-yet-activated processes. Sleeping and killed processes are excluded.
Query with zero capacity to get the required count. With nonzero insufficient
capacity, it returns `MV_BUFFER_TOO_SMALL`, writes the required count and leaves
the record buffer unchanged. NULL context/count, or NULL records with nonzero
capacity, is invalid and leaves outputs unchanged.

The caller owns the buffer. Sorting is in place, takes O(n log n), and allocates
no internal storage. The query does not tick, draw, call user code or retain any
process or resource. The copied order can become stale after later mutations.

## Lua

`self.z` reads and writes native depth. Process defaults and spawn options keep
their existing `z` behavior. Invalid assignments raise an error without mutation.
Lua keeps the original integer/float representation for readback while it
matches the native effective value. The rendering key is always the native
double, as in the previous Lua sorting path. A native mutation becomes visible
to Lua getters. Presentation metadata is not a second source for ordering.

The runner uses the native ordered snapshot. It still draws a process graph
before that process's queued commands. Frozen processes remain visible and
sleeping processes hide. Persistent text and fades keep their later draw passes.
The Lua slot lookup uses increasing creation IDs without another slot map.

## Rust

Use `handle.z()?`, `handle.set_z(value)?`, and `engine.draw_order()?`.
The ordered result contains owned handles. These handles do not retain processes
or resources. A process can be gone when later inspected. After engine shutdown,
depth operations return `Closed`. During a process, use `process.current()` to
get its handle. No graphics feature or window is required.

```rust,no_run
use malvado::{Engine, Error};
let engine = Engine::new()?;
let actor = engine.spawn(Default::default(), |p| async move {
    loop { p.frame().await; }
})?;
actor.set_z(-2.5)?;
assert_eq!(actor.z()?, -2.5);
let visible = engine.draw_order()?;
assert_eq!(visible[0].id(), actor.id());
# Ok::<(), Error>(())
```

Angle, scale, process graph storage and automatic draw-command
collection remain open in A7.6. An ordered handle query is not a renderer.

## Verification

`process_draw_order` checks native defaults, extreme finite values, invalid and
foreign references, dead processes, output preservation, small/NULL buffers,
ties, sleep/freeze/wake/kill and a fixed five-phase order. Rust tests verify the
same phases, scheduler-priority independence, closed handles, copied order and
257 processes with duplicate/fractional depths against a reference sort.
`lua_process_z` checks integer/float readback and invalid assignments.
`visual_native_process_order` verifies a C-to-Lua-to-C mutation, rendered order,
sleeping exclusion and frozen visibility. Existing visual draw-order fixtures
cover ties and later process drawings. Installed signature/layout checks,
Debug/Release, a relocated consumer and native sanitizers cover the Rust SDK.

## Native position storage

`mv_process_position(ctx, reference, &x, &y)` copies the native coordinates.
Outputs must be non-NULL and distinct. `mv_process_set_position(ctx, reference,
x, y)` sets both coordinates atomically. New processes start at `(0, 0)`.
Coordinates must be finite; negative and extreme finite coordinates are accepted.
Both functions use the depth API identity and error rules. Failure preserves
state and outputs. Ticks do not change the stored position. No allocation,
rendering or collider update occurs.

Lua `self.x` and `self.y` share valid numeric coordinates with this native
storage. Spawn/default values, assignments and world movement update C.
Getters, graph rendering, camera follow, world queries and Lua process collision
queries read native changes, including for sleeping or frozen targets. No Lua
activation is required before a graph reflects a native move. Direct C collider
queries still use the transform explicitly registered with the collider API;
the Lua adapter refreshes its derived colliders before Lua queries.

Lua preserves integer/float presentation while its value matches C. Setting
`nil` resets that coordinate to zero. To preserve the existing Lua visual error
contract, non-number and non-finite assignments remain local invalid overrides.
They do not change the last valid native coordinate. Native writes do not clear
these overrides. Assign a finite number or `nil` in Lua to clear one. Rendering,
world queries and colliders retain their existing handling of invalid fields.
Numeric strings retain their existing per-operation conversion behavior.
`process_position` verifies defaults, extremes, invalid inputs, output aliasing,
foreign/dead references, tick persistence and atomic failure.

Rust exposes this storage through `handle.position()?` and
`handle.set_position(x, y)?`. Queries return a copied `(f64, f64)` tuple.
Cloned handles observe the same native process. A killed process returns
`NotFound`; a dropped engine returns `Closed`. Sleeping and freezing preserve
position. `tests/position.rs` checks these rules and failed writes; the external
installed SDK consumer checks a fractional-coordinate round trip. Both native
signatures are checked against the installed headers before linking.

Host verification for A7.6x/y: `mise run test-c` (56 tests),
`mise run test-headless` (170 tests), `mise run test` (170 tests),
`mise run test-rust` (Debug/Release, Clippy, formatting and relocated consumer),
and `mise run test-rust-sanitize` all pass on macOS. Linux CI and automatic
visual integration are not certified by these local checks.

`native_lua_process_position` checks C/Lua round trips, native world-move
writeback, collider refresh, integer presentation, nil reset and invalid Lua
overrides. `visual_native_process_position` checks pixels before and after a
native move of a frozen Lua process. Existing camera, world, graph and collision
fixtures cover the unchanged Lua contracts.
