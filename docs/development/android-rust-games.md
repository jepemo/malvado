# Android Rust game packages

Rust games can now use the same installed Android session runner as C and Lua.
They keep the safe `Game` API and the existing native process scheduler. The
Android entry is a small bridge; it does not install another event loop.

Add a static library target to a normal Cargo package:

```toml
[lib]
name = "game"
path = "android.rs"
crate-type = ["staticlib"]
```

Keep the ordinary `fn main()` in `main.rs`. Add this sibling `android.rs`:

```rust
#![cfg(target_os = "android")]
include!("main.rs");
malvado::android_entry!(main);
```

Use a relative include path that matches your project layout. Source files used
with `include!` must be valid in that context; move crate-level attributes to the
bridge when required. Invoke the macro once in the Android library. The bridge
is excluded on desktop targets, which keep the ordinary binary entry.

The entry takes no arguments. It accepts `()` or another Rust `Termination`
return type, such as `Result<(), Error>`. Success maps to native status 0. Other
termination values, returned errors and unwinding panics map to 1. The macro
contains unwinding panics before returning through C. A `panic=abort` profile
still aborts the process and cannot provide ordinary cleanup evidence.

Release owned games, resources and windows before the entry returns. Normal
Rust drop behavior also applies during unwinding. The shared runner logs stderr
and status, stages ordinary assets into a private session and removes that
session on return. Static state persists within the process. Persistent saves
and process-death restoration have the [shared runtime](android-runtime.md)
limits.

For SDK consumers that build manually, find the installed Android `graphics`
component and call `malvado_add_android_rust_game(game ARCHIVE_PATH)` after Cargo
produces the Android static library. The function links the archive with the
shared runner and SDK graphics target, producing `libgame.so`. It rejects a
missing archive. Rebuild and reinstall older SDK prefixes to get this helper
and `android_entry!`.

The actual Debug and Release arm64 game checks include native pixels, binary
assets, process-owned image cleanup, a returned process error and a contained
panic. Rust host tests also call the C entry for success, error and panic and
verify exactly-once drop behavior. This is production entry support, not a
claim of full alpha.8 acceptance on every host.
