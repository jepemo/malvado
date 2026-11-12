# Owned text content

Include `malvado/text.h` and link the core. `MvText` owns a copied
NUL-terminated string, integer screen coordinates and horizontal alignment.
It needs no window, font or process. Use it to keep text between activations.

```c
MvText *text = NULL;
if (mv_text_create("Score: 0", 10, 10, 0, &text) == MV_OK) {
    /* A failed replacement leaves the complete previous value intact. */
    MvResult result = mv_text_update(text, "Score: 1", 10, 10, 0);
    (void)result;
    mv_text_destroy(text);
}
```

Alignment is 0 for left, 1 for center or 2 for right. Use
`mv_text_aligned_x` with the measured font width when rendering.
`mv_text_read` returns a borrowed string and copied coordinates/alignment.
All output pointers are required. The borrowed string lasts until update or
destruction. Update can use the object's own borrowed string, including a suffix.
Empty strings and the complete `int` coordinate range are valid.

NULL arguments or invalid alignment return `MV_INVALID_ARGUMENT`. Allocation
failure returns `MV_OUT_OF_MEMORY`. Failure leaves outputs and stored values
unchanged. Create transfers one object to the caller only on success. Call
`mv_text_destroy` once for each object; NULL destruction is harmless. Keep
access and destruction on the owner thread. C content is bytes, with no UTF-8
validation. The terminating NUL is not part of the visible content.

This service does not retain a font, register a process resource, collect text
objects or draw them. These remain explicit host responsibilities. It does not
change the scheduler or introduce automatic process ownership.

## Lua persistent text

`mv.write`, `mv.change_text` and `mv.delete_text` keep their existing signatures.
Content, position and alignment use the native owned object. Lua retains the
font descriptor and the existing creation order. A font replacement changes
only the selected descriptor; a failed argument check leaves all fields intact.
Deletion and process cleanup release native content immediately. Later garbage
collection is harmless. Host text has no process owner and lasts until deletion
or runtime shutdown. Process text is removed when its owner returns or is killed.
Sleep and freeze do not remove it. The font table remains live, as before.

## Rust owned text

`malvado::text::Text` owns the same native object. `Text::new` and `update`
copy UTF-8 content and accept a position tuple and `graphic::TextAlignment`.
An embedded NUL returns `Error::InvalidArgument`; allocation failures are
explicit. `snapshot()` borrows the content and copies position/alignment.
Rust prevents update or destruction while that borrowed content is in use.
The owner stays on its creating thread and releases native memory on Drop.
See the runnable example on `Text::new`.

This is content storage, without implicit process registration or drawing.
A shared native text list with retained fonts and automatic rendering remains
unfinished in A7.6.

## Context text registry

`mv_text_entry_create` copies content into a context-owned entry. Pass a live
`MvProcessRef` as owner, or NULL for host text. The returned positive ID belongs
to that context. IDs increase through `INT64_MAX` and are never reused. Do not
use an ID with a different context. Failed creation leaves the output and the
next ID unchanged.

Use `mv_text_entry_read`, `mv_text_entry_update` and `mv_text_entry_remove` with
that context and ID. Read borrows content until update, removal, owner cleanup
or shutdown. Update is atomic and can use the entry's borrowed content. Missing
IDs return `MV_NOT_FOUND`. Invalid arguments return `MV_INVALID_ARGUMENT`.
Creation can also return `MV_OUT_OF_MEMORY`, `MV_OVERFLOW` or `MV_BUSY` during
shutdown. All calls stay on the context's owner thread.

`mv_text_entries` copies IDs in creation order. Query the required count with
NULL and capacity 0 first. A short buffer returns `MV_OVERFLOW`, writes the
required count and leaves the ID buffer unchanged. Enumeration allocates nothing.

Sleep and freeze retain process text. Kill and natural return remove it at the
scheduler cleanup point, after process destructors and generic resource cleanup.
A destructor can still read or explicitly remove its text. Host entries remain
until removal or context shutdown. Shutdown frees every remaining entry.
The registry does not retain fonts or draw entries.

## Lua registry integration

Lua `write` now creates an entry in the native context registry. `change_text`
and rendering read that entry, so native content or position changes are visible
to Lua. A native removal makes subsequent Lua changes/deletes fail as unknown
text IDs. Process cleanup removes native entries and releases Lua font roots;
GC no longer owns a separate text object. Existing signatures, font table
identity and rendering order are preserved.

The renderer still uses Lua's font descriptors. Entries created only by C are
not automatically rendered by Lua. Native font retention and common drawing
collection remain unfinished. The registry itself works without Lua or graphics.

## Rust context text handles

Use `Engine::create_text` for host-owned entries and `Handle::create_text` for
process-owned entries. Both return `TextHandle`. Its clones refer to the same
context entry; dropping a handle does not remove it. Call `remove` to remove it
explicitly. Process cleanup and engine shutdown remove entries automatically.
This follows the native registry lifetime, independently of Rust handle lifetime.

`update` copies content and `snapshot` returns an owned string, position and
alignment. Owned snapshots remain valid across updates, ticks and shutdown.
Handles remain local to one thread. Missing entries return `Error::NotFound`;
a closed engine returns `Error::Closed`. Embedded NUL is rejected before updates.
The existing standalone `Text` retains its explicit Drop-owned lifetime.
Neither service registers a font or draws automatically.

## Native font references

`mv_font_load` and `mv_font_load_codepoints` return one owned reference.
`mv_font_retain` acquires another reference to the same immutable native font.
It returns 0 for NULL or reference-count overflow without changing ownership.
Release each acquired reference with `mv_font_destroy`. The final release unloads
the glyph texture. When a frame is active, that release flushes queued drawing
before unloading the texture. NULL destruction is harmless.

All references stay on the window owner thread. They do not retain the window;
release them before window close. Headless retain returns 0. Rust's safe `Font`
continues to share its native owner and window through its existing Rc wrapper.
The unsafe sys SDK also declares native retain.

## Native retained text drawing

`mv_text_entry_font(ctx, id, font, size, color)` gives a registry entry a drawing
style. Size is 1..1024. Color and size are copied. Pass NULL for the built-in font,
or a live custom font to acquire one native reference. The caller keeps its own
reference and can release it after a successful bind. Replacing a style acquires
the new reference before releasing the old one. Failed validation or allocation
preserves the complete previous style and caller ownership.

`mv_text_entry_font_clear` removes the style without changing content, position
or alignment. It is idempotent for an existing entry. Removal, owner cleanup and
context shutdown release its retained font. Invalid size/context returns
`MV_INVALID_ARGUMENT`, a missing entry returns `MV_NOT_FOUND`, and allocation
failure returns `MV_OUT_OF_MEMORY`. All access stays on the window/context owner
thread. Retained fonts do not retain the window; remove or clear their entries
before closing it.

Call `mv_window_draw_texts(ctx)` between window begin/end, with no active clip or
scroll scope. It draws styled entries in creation order, uses screen coordinates
and the shared measured-width alignment, and skips unstyled entries. It allocates
nothing and does not tick processes or change content. No frame/window, an active
scope, or a NULL context returns 0 without drawing. The headless backend returns
0. The host chooses the order, for example process graphics, persistent text,
then the fade overlay. Entries are unstyled until explicitly bound.

```c
uint64_t id;
if (mv_text_entry_create(ctx, NULL, "Score: 1", 10, 10, 0, &id) == MV_OK) {
    MvResult result = mv_text_entry_font(ctx, id, NULL, 20,
                                       (MvColor){255, 255, 255, 255});
    (void)result;
}
/* Inside an active frame, without clip/scroll scopes: */
(void)mv_window_draw_texts(ctx);
```

Lua still uses live font tables and its current text rendering pass. Safe Rust
text handles do not yet bind fonts or retain a window for this pass. The unsafe
sys SDK exposes these native functions with installed signature checks. Their
binding integration remains unfinished in A7.6.
