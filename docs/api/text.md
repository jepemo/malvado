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
