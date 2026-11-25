# FPG graphics

Run from the repository root with graphics and assets enabled:

```sh
./build/malvado run examples/fpg/main.lua
```

The installed CLI also runs the example with `malvado run .` from this
directory or with `malvado run /path/to/main.lua` from any directory.

The example loads an original 8-bit FPG, uses `graph()` for IDs 7 and 42,
and uses the first control point as each graphic pivot. Index 0 is transparent.
Press Escape to exit. The fixture is generated from
`tests/fixtures/fpg/generate.py` and uses the project license. See the
[FPG contract](../../docs/api/fpg.md) for supported variants and limits.
