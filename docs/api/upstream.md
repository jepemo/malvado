# Pinned reference sources

These files are external originals that can be downloaded for API research. They are not tracked in Git and are not Malvado build dependencies.

- BennuGD: `d49f71891f38db9bbf08c0f6f0002007f564a03c`.
- BennuGD2: `baf8dc1b744d47dae58fc20c139dd44656907911`.
- [Manifest with URL, original path, and SHA-256](upstream-manifest.json).
- Original notices: [BennuGD modules/COPYING](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/COPYING), [BennuGD2 LICENSE.md](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/LICENSE.md), and each file header. Keep these notices in any redistribution. Do not attribute this code to Malvado.

The download preserves the original encoding and line endings. The derived catalog is new. You can read it without downloading or building the sources. It contains interface metadata, not translations of implementations.

## Reproduce

```sh
python3 docs/tools/fetch_api_sources.py
python3 docs/tools/catalog_api.py
```

The generator checks hashes. It includes all `*_exports.h` files from the source trees: 45 in BennuGD and 10 in BennuGD2. Auxiliary files are not counted as export headers. To update, first obtain the tree and SHA of the new commit. Download all its exports, update the manifest, and review the generated diff in a PR. Never download from a mutable branch without pinning the result.

The generator is a lexical extractor, not a C preprocessor. It includes alternative branches. `preprocessor_context` retains each level of directives so that `else/elif` can be interpreted without claiming that conditions were evaluated. Constants retain expressions rather than resolved numeric values. For global and local blocks, `raw_initializer` retains the initializer without comments and its macros. `declaration_string_fragments` concatenates only literals and **does not expand macros**. It is not a substitute for the original.

`FUNC` signatures are stored literally (`I`, `D`, `S`, `P`, `B`, etc.). Check the relevant revision to translate types, widths, and output parameters. Do not assume that one character always represents one public Lua argument. Aliases and overloads remain separate rows. Variable blocks are retained in full. Each structure field is not counted as a separate symbol.

The auxiliary `sysprocs*` and `instance.c` files support core research. They are not a complete extraction of its contract. There is no snapshot of all DIV/Fénix or third-party extensions.
