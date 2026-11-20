# Maintain the API documentation

Public C headers, the Lua export inventories and Rust crate sources define
names and types. The [API guides](../api/README.md) describe behavior,
ownership, errors and platform limits. Update the guides, inventory entries,
tests and relevant runnable examples when the public API changes.

## Catalog and link checks

Run `mise run catalog` to fetch pinned upstream sources, verify their hashes
and regenerate the reference inventory. Keep this catalog separate from
the Malvado API.

Run `mise run test-docs` to verify local links, heading anchors and GitHub
links to current repository files. The [publishing guide](publishing.md)
covers the Astro build and versioned snapshots.

## Executable Rust guide checks

`tools/test_rust_docs.py` reads every Rust block in `docs/api/*.md`, builds each
as a main-thread executable against installed `malvado` crate sources, then runs
it with a bounded timeout. It uses the actual guide text, including Rustdoc
hidden setup lines. No ignored or compile-only Rust blocks are accepted. The
fixture supplies existing licensed PNG, font, FPG, WAV and OGG inputs in a
temporary directory; no teaching example or game asset catalog is added.

`mise run test-rust-graphics` includes these checks for both SDK profiles and
hosts with a display. For an existing graphical SDK, run:

```sh
mise exec -- python3 tools/test_rust_docs.py --sdk "/path/to/installed sdk"
mise exec -- python3 tools/test_rust_docs.py --sdk "/path/to/installed sdk" --release
```
