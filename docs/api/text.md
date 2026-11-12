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
