# Rust integration migration plan

Status: proposal; begin only after alpha.6 acceptance and owner review. No engine or directory migration is authorized by this plan.

1. Inventory native symbols, installed headers, ABI, build options, existing `src/` paths, CLI templates and resource contracts. Establish a C/Lua trace baseline and identify missing public operations.
2. Create `bindings/rust/` only when implementation starts. Keep CMake as native owner and Cargo as Rust owner. Install a C SDK to a temporary prefix for CI and external consumers. Add optional `mise run test-rust`; preserve existing C/Lua commands without invoking Cargo.
3. Prototype the scheduler and cancellation in isolation before committing macros or user-facing syntax. Add minimal public C hooks only with ownership and failure contracts; test the C-only build after each change.
4. Wrap accepted alpha.6 graphics, collisions and resources through the native API. Compare shared fixtures and document any unavailable native API instead of inventing it.
5. Extend the existing C client with Rust templates and Cargo invocation. Preserve `new --c` and current Lua detection. Specify `--lang` conflict handling and ambiguous project discovery before implementing; do not silently change Lua precedence. Prefer Cargo metadata and add `malvado.toml` only for information Cargo cannot express.
6. Add tests for spaces, missing Cargo/compiler/SDK, invalid manifests, incremental rebuilds, and clean installed SDK use. `doctor` reports optional capabilities without breaking Lua use; no automatic toolchain installation. Define build/check/run/export errors and exit codes in the CLI spec before implementation.
7. Ship native host exports with assets, libraries and licenses; validate on a clean automated runner without source paths or development tools. Cross compilation, Windows and Android support require their own verified target matrix in alpha.9.

Keep current source, CLI and examples in place; moving them into `engine/` or `cli/` later requires a separate decision and compatibility plan. For alpha.10 preserve the independent website foundation and plan a language selector without rewriting it now. Existing historical alpha links cannot safely redirect to their old meaning after renumbering: review published links at site migration, add explicit migration notices/redirects where feasible, and document ambiguity.
