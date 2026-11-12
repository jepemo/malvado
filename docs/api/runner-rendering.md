# Shared process rendering

`mv_window_draw_process_graphics(ctx)` draws the context's native process snapshot
inside an active frame. It draws each selected graph before that process's stored
commands, in ascending z and creation order. Sleeping processes are hidden;
frozen processes stay visible. A bad graph transform suppresses the graph while
its independent drawing commands remain visible.

Camera selection, viewport/region intersections and scroll offsets use the native
services. The pass restores the caller's clip and scroll scopes. It does not
advance time, activate processes or change graph assignment. It flushes borrowed
texture uses before returning. The headless backend rejects drawing.

Rust `frame.process_graphics(&engine)` calls this same pass. It requires the
existing owner-thread window and engine. Its installed native signature is
unchanged.

## Binding preparation

The platform has a private preparation entry point for language bindings.
It snapshots IDs/order first, then lets a binding prepare each process before
reading its native graph and transform. A binding can suppress that graph without
suppressing its stored commands. The hook is not an installed public API.

Preparation can synchronize descriptors and release resources. It must not tick,
draw, enter graphics scopes, destroy the context or close platform services.
The pass flushes queued drawing before each callback. A callback can therefore
release the last reference to a previously queued image safely. Native values
are read again after preparation, and no callback or user pointer is retained.
The public C/Rust path prepares all graphs unchanged.

Automated native pixels check graph suppression with visible commands, transforms
changed during preparation, and final queued-image release. Invalid-frame and
headless calls must not invoke preparation. Existing native visual tests cover
projection, clipping, scroll restoration, sleep/freeze and invalid geometry.
