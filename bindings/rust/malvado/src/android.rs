// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! NativeActivity entry for ordinary Rust game code.

/// Export a Rust entry for the Android SDK runner.
///
/// Invoke this once in an Android `staticlib` target. The entry takes no
/// arguments and returns `()` or another [`std::process::Termination`] type,
/// such as `Result<(), Error>`. It must release its game/window before return.
/// Success maps to 0; errors and unwinding panics map to 1. `panic=abort` still
/// aborts the process. No panic unwinds across the C boundary.
///
/// ```ignore
/// #![cfg(target_os = "android")]
/// include!("main.rs");
/// malvado::android_entry!(main);
/// ```
#[macro_export]
macro_rules! android_entry {
    ($entry:path) => {
        #[unsafe(no_mangle)]
        pub extern "C" fn malvado_game_main() -> ::std::ffi::c_int {
            match ::std::panic::catch_unwind(|| ::std::process::Termination::report($entry())) {
                Ok(status) if status == ::std::process::ExitCode::SUCCESS => 0,
                _ => 1,
            }
        }
    };
}

#[cfg(test)]
mod tests {
    use std::sync::atomic::{AtomicUsize, Ordering};
    static MODE: AtomicUsize = AtomicUsize::new(0);
    static DROPS: AtomicUsize = AtomicUsize::new(0);
    struct Guard;
    impl Drop for Guard {
        fn drop(&mut self) {
            DROPS.fetch_add(1, Ordering::SeqCst);
        }
    }
    fn game() -> Result<(), &'static str> {
        let _guard = Guard;
        match MODE.load(Ordering::SeqCst) {
            0 => Ok(()),
            1 => Err("expected entry error"),
            _ => panic!("expected entry panic"),
        }
    }
    crate::android_entry!(game);
    #[test]
    fn native_entry_contains_errors_and_unwinding_panics() {
        for mode in 0..3 {
            MODE.store(mode, Ordering::SeqCst);
            assert_eq!(malvado_game_main(), if mode == 0 { 0 } else { 1 });
            assert_eq!(DROPS.load(Ordering::SeqCst), mode + 1);
        }
    }
}
