# Managed native platform resources

Use the native resource manager when a loaded image, font, sound or music stream
belongs to a process or game context. Lua loads and sound clones use these same
constructors. C hosts no longer need to define destructor callbacks for them.

```c
MvImage *image = mv_image_load("player.png");
MvResource *resource;
if (image) {
    MvResult result = mv_resource_adopt_image(ctx, &actor, image, &resource);
    if (result != MV_OK) {
        mv_image_destroy(image); /* Failed adoption leaves ownership here. */
    }
    /* On success, actor owns the resource. Retain an independent use before
     * allowing actor to end. Destroy the image only through the resource. */
}
```

`mv_resource_adopt_image`, `mv_resource_adopt_font`, `mv_resource_adopt_sound`
and `mv_resource_adopt_music` transfer one caller-owned native reference on
success. They return a borrowed resource pointer. NULL owner selects the context;
a process owner must be live in that same context. The resource manager calls
the correct native destructor once after the owner and all retained uses end.
`mv_resource_data` returns the original typed pointer as `void *`.

Each successful image load supplies one owned reference, even when the path
cache returns the same pointer. Adopt each load reference at most once. Sound
clones remain independent playback instances and each has its own owner.
Adoption does not load, clone, start, stop, pause or update playback.

NULL data/context/output and foreign owner references return
`MV_INVALID_ARGUMENT`; a dead owner returns `MV_NOT_FOUND`. Closing contexts
return `MV_BUSY`, and allocation failure returns `MV_OUT_OF_MEMORY`.
Failure preserves the output and leaves data with the caller. The caller must
release failed loads. Adoption does not add an independent retained reference.
Use `mv_resource_retain`/`mv_resource_release` for each independent use and
`mv_resource_transfer` to change the owner. All calls and destruction stay on
the owner thread.

Native references do not keep a window or audio device open. Release all native
objects before closing their platform service. A reference can outlive its
context only while its underlying service remains live. Core-only engines do
not gain a graphics or audio dependency; the adoption functions belong to the
platform library. The headless library can link them but cannot load platform
objects.

Rust exposes the native constructors in unsafe `malvado_sys::graphics`, with
installed-header signature checks. Raw adoption has the same caller obligations
as C. Safe Rust programs use the existing `Game::resource(value)` or
`Process::resource(value)` with owned `Image`, `Font`, `Sound` and `Music` values.
`Resource<T>` uses the same native owner/retain/transfer manager. Its Rust value
retains the window/audio service until destruction; no raw pointer adoption is
needed. A resource can be transferred between live same-context owners. The
[resource contract](resource-lifetime.md) defines cleanup and shutdown order.

`platform_resource_adoption` uses fake native objects to count destruction
exactly. It checks rejected/NULL/foreign/dead adoption, unchanged failure output,
process cleanup, transfer to context, independent retention and release after
context shutdown. Rust `managed_platform` uses real cached images, custom fonts,
sound clones and music with the installed SDK. It checks independent cached
load ownership and live retained font/music use after process/context cleanup.
Existing Lua graphics/audio/resource tests cover the migrated load paths.
