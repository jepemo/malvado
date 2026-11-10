# Rust integration migration plan

Status: in progress after alpha.6 acceptance. Native inventory and installed sys SDK are complete; see [alpha.7](../versions/2.0.0-alpha.7.md). The scheduler proof is next. The owner also approved the bounded Lua binding relocation in A7.11. No wider engine or directory migration is included.

1. Inventory native symbols, installed headers, ABI, build options, existing `src/` paths, CLI templates and resource contracts. Establish a C/Lua trace baseline and identify missing public operations.
2. Create `bindings/rust/` only when implementation starts. Keep CMake as native owner and Cargo as Rust owner. Install a C SDK to a temporary prefix for CI and external consumers. Add optional `mise run test-rust`; preserve existing C/Lua commands without invoking Cargo.
3. Prototype the scheduler and cancellation in isolation before committing macros or user-facing syntax. Add minimal public C hooks only with ownership and failure contracts; test the C-only build after each change.
4. Wrap accepted alpha.6 graphics, collisions and resources through the native API. Compare shared fixtures and document any unavailable native API instead of inventing it.
5. Extend the existing C client with Rust templates and Cargo invocation. Preserve `new --c` and current Lua detection. Specify `--lang` conflict handling and ambiguous project discovery before implementing; do not silently change Lua precedence. Prefer Cargo metadata and add `malvado.toml` only for information Cargo cannot express.
6. Add tests for spaces, missing Cargo/compiler/SDK, invalid manifests, incremental rebuilds, and clean installed SDK use. `doctor` reports optional capabilities without breaking Lua use; no automatic toolchain installation. Define build/check/run/export errors and exit codes in the CLI spec before implementation.
7. Ship native host exports with assets, libraries and licenses; validate on a clean automated runner without source paths or development tools. Cross compilation, Windows and Android support require their own verified target matrix in alpha.9.

Keep current source, CLI and examples in place; moving them into `engine/` or `cli/` later requires a separate decision and compatibility plan. For alpha.10 preserve the independent website foundation and plan a language selector without rewriting it now. Existing historical alpha links cannot safely redirect to their old meaning after renumbering: review published links at site migration, add explicit migration notices/redirects where feasible, and document ambiguity.

## Lua binding relocation (A7.11)

Status: relocated to `bindings/lua/`; acceptance checks remain in A7.11. Complete after A7.2 and before
changing Lua binding behavior for A7.6 or validating final packages in A7.9.
This item does not block the independent Rust scheduler proof in A7.3.

1. Audit references to `src/binding/` and includes of binding headers in CMake,
   source, tests, tools, CI, and documentation. Record the existing targets and
   installed paths so the move preserves them.
2. Move the existing Lua adapter files to `bindings/lua/`, preserving their
   contents and license headers. Keep shared core and platform code in `src/`.
   Do not split `runtime.c` or extract shared behavior in the relocation commit.
3. Update source lists, private include paths, inventory readers, packaging
   scripts, and current documentation links. Keep public include paths, target
   names, CLI behavior, and exported SDK paths unchanged. Historical evidence
   may retain the original path if it clearly describes the earlier revision.
4. Verify `mise run test`, `mise run test-c`, `mise run test-headless`,
   `mise run test-rust`, and `mise run test-visual`. Check API inventories,
   deterministic C/Lua traces, installed SDK consumers, and existing package
   checks. Verify C-only and Lua builds with Cargo absent from PATH.
5. Review remaining old-path references and macOS/Linux CI results. Record the
   evidence and check A7.11 only when the complete relocation and required
   checks pass. Keep this as a focused commit separate from API implementation.

CMake continues to build Lua; Cargo continues to manage Rust. The shared
directory does not require matching internal layouts or another build layer.
