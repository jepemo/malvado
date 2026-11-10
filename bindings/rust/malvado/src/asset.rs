// Malvado 2; SPDX-License-Identifier: Apache-2.0
use std::sync::{Mutex, MutexGuard};

// cJSON writes one global error record even when callers do not read it.
// Serialize safe Rust JSON import calls; no user callbacks run under this lock.
static JSON_PARSE: Mutex<()> = Mutex::new(());

pub(crate) fn json_guard() -> MutexGuard<'static, ()> {
    // No Rust data is shared behind the guard. Poison does not invalidate the
    // native parser, whose error record is reset on its next call.
    JSON_PARSE
        .lock()
        .unwrap_or_else(|poison| poison.into_inner())
}
