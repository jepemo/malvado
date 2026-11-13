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

This is standalone content storage. Use context text handles for retained fonts
and the shared native drawing pass.

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

The runner synchronizes live Lua font descriptors, then draws the shared native
registry. Styled entries created only by C are included in creation order.
Unstyled entries remain invisible. The registry works without Lua or graphics.

## Rust context text handles

Use `Game::create_text` for host-owned entries and `Handle::create_text` for
process-owned entries. Both return `TextHandle`. Its clones refer to the same
context entry; dropping a handle does not remove it. Call `remove` to remove it
explicitly. Process cleanup and engine shutdown remove entries automatically.
This follows the native registry lifetime, independently of Rust handle lifetime.

`update` copies content and `snapshot` returns an owned string, position and
alignment. Owned snapshots remain valid across updates, ticks and shutdown.
Handles remain local to one thread. Missing entries return `Error::NotFound`;
a closed engine returns `Error::Closed`. Embedded NUL is rejected before updates.
The existing standalone `Text` retains its explicit Drop-owned lifetime.
Context handles support explicit font assignment and frame drawing with the
optional graphics feature; neither service advances a scheduler.

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

Lua prepares styles from its live font tables and uses this native pass.
Rust context text handles retain their font/window owner and expose explicit
frame drawing. The sys SDK checks the installed native signatures.

## Lua shared text drawing

Lua `write`, `change_text` and `delete_text` keep their signatures. Before drawing,
the runner synchronizes each live font table's resource, size and color into the
native entry. Valid table changes remain visible, including changes after a
process has slept or frozen. A failed allocation leaves the previous style intact.
Unchanged styles reuse their allocation and font reference. Invalid font sizes
outside the descriptor's 1..1024 contract suppress that entry's drawing.

One native pass draws all styled entries in creation order after process graphics
and before the fade overlay. This includes C-created host entries. Lua no longer
measures, aligns or dispatches persistent text drawing independently. Descriptor
roots remain in Lua so tables stay live; native references protect custom fonts
through replacement, deletion, process cleanup and shutdown. Headless behavior
and text content APIs are unchanged.

## Rust retained text drawing

With the graphics feature, `TextHandle::set_font(&font, size, color)` retains a
custom `Font` and its window. `set_builtin_font(window.clone(), size, color)`
retains the window for the built-in font. Both copy size/color into the native
style; size must be 1..1024. A failed assignment preserves the previous style.
`clear_font()` releases the style without deleting content. Cloned text handles
share the assignment. Dropping a handle does not clear a context-owned entry.

The native style owns the Rust lifetime token until replacement, clear, removal,
process cleanup or engine shutdown. Cleanup releases the native font reference
before dropping the token. Failed calls keep the token with Rust and release it
there. Callbacks contain destructor panics so none unwind through C.
`mv_text_entry_font_owned` exposes this optional lifetime token to C bindings;
pass both owner and destroy, or neither. It transfers the token only on success.

Call `frame.texts(&engine)` after `frame.process_graphics(&engine)` and before
an optional fade overlay. This is explicit drawing, with no extra scheduler or
implicit process polling. Active clip/scroll scopes return an error without
drawing. Missing entries return `Error::NotFound`; closed engines return
`Error::Closed`. Styled entries retain their window even after the caller drops
its original font/window variables, and release it when the final style ends.

```rust
use std::rc::Rc;
use malvado::{Game, graphic::TextAlignment, window::{Window, Color}};
let window = Rc::new(Window::open(320, 200, "Text")?);
let game = Game::new()?;
let text = game.create_text("Score: 1", (10, 10), TextAlignment::Left)?;
text.set_builtin_font(window.clone(), 20, Color {r:255,g:255,b:255,a:255})?;
let frame = window.frame(Color {r:0,g:0,b:0,a:255})?;
frame.texts(&game)?;
drop(frame);
text.remove()?;
# Ok::<(), malvado::Error>(())
```
