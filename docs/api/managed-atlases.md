# Managed atlas metadata

`mv_resource_adopt_atlas(ctx, owner, atlas, &resource)` transfers a parsed,
caller-owned atlas into the native resource manager. This is useful when a
process or game context must own metadata. The call needs the optional assets
library and no window or Lua runtime.

The constructor selects the native atlas destructor. Success returns a borrowed
resource pointer, with one owner reference and no independent retained use.
Retain each independent use before its owner can end. Transfer and release use
`mv_resource_transfer`, `mv_resource_retain` and `mv_resource_release`.
The atlas, its names and animation metadata remain valid until final release.
Context shutdown releases its owner reference; retained metadata can outlive it.
An atlas owns no image or graphics device. Retaining metadata does not retain
its image. A renderer must keep image uses separately.

NULL data/context/output and foreign owners return `MV_INVALID_ARGUMENT`.
Dead owners return `MV_NOT_FOUND`; closing contexts return `MV_BUSY`.
Allocation failure returns `MV_OUT_OF_MEMORY`. Failure leaves the output and
caller ownership unchanged. Destroy a rejected atlas directly. After success,
destroy it only through its resource. Transfer each owned atlas once. Access
and cleanup use the context owner thread, including release after shutdown.

Lua atlas and Aseprite loads, including unified `load_asset`, use this native
constructor. The Lua wrapper still holds its image and converts Lua arguments.
This change does not move unified file format dispatch or graph selection into
C. Rust exposes unsafe adoption in `malvado_sys::assets`, with installed signature
checks. Safe Rust can already place an owned `Atlas` in `Engine::resource` or
`Process::resource`, and transfer it through the native resource manager.

`atlas_resource` parses real metadata, rejects invalid/foreign/dead calls,
checks unchanged failure output, transfers a process owner to the context,
and reads a retained sprite after context shutdown. It also exercises context
cleanup without independent uses. Rust atlas tests call the same constructor
and read retained metadata after shutdown. Installed assets consumers check
linking the new assets-to-core dependency without graphics, Lua or source paths.
Existing Lua atlas/Aseprite and resource visual checks cover migrated paths.
