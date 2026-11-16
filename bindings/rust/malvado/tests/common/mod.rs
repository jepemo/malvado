// Malvado 2; SPDX-License-Identifier: Apache-2.0
pub fn root() -> std::path::PathBuf {
    std::env::var_os("MALVADO_TEST_ROOT")
        .map(std::path::PathBuf::from)
        .unwrap_or_else(|| std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../.."))
}
