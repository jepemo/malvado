# DIV family reference

Research date: 2026-10-29. Design recommendation: use **BennuGD2 as the main modern reference**, BennuGD classic as the historical inventory, and DIV/Fénix as semantic references. They are not interchangeable APIs or one standard.

## Sources and choice

| Reference | Primary source | Proposed use |
| --- | --- | --- |
| DIV Games Studio | [Code, help, and examples](https://github.com/DIVGAMES/DIV-Games-Studio), revision `77556912335d0a1b0efea855d4eb2490cf382128` | Original process and format model; historical comparison; help in `help/`, runtime in `src/` |
| Fénix | [Project distributions](https://sourceforge.net/projects/fenix/files/Fenix/) | Check historical differences; the consulted page lists 0.92a. No complete Fénix 0.93 snapshot is pinned here |
| BennuGD classic | [SplinterGU repository](https://github.com/SplinterGU/BennuGD), revision `d49f71891f38db9bbf08c0f6f0002007f564a03c` | 45 module export headers; useful modular historical inventory |
| BennuGD2 | [SplinterGU repository](https://github.com/SplinterGU/BennuGD2), revision `baf8dc1b744d47dae58fc20c139dd44656907911` | 10 export headers; main reference for a modern game API |
| BennuGD documentation | [Project portal](https://divhub.github.io/bennugd-website/docs/) and [historical manual](https://www.bennugd.org/downloads/static_docs/General.pdf) | Explanations and examples; always check against the source revision |
| PixTudio and other variants | [DIV Project Manager](https://divpm.divhub.org/), which documents several runtimes | Candidates for later comparison; their APIs were not captured and do not define the initial contract |

The choice of BennuGD2 is a design conclusion based on its modules and code. It does not guarantee compatibility or claim that it covers every DIV function. For example, BennuGD classic has `mod_m7`, `mod_path`, `mod_flic`, `mod_cd`, `mod_blendop`, and `mod_effects`. The modern layout groups many functions in `libmod_gfx`, `libmod_misc`, `libmod_input`, and `libmod_sound`, and includes network, ads, and IAP modules. Compare both names and semantics. Do not equate modules by count.

## Results

The 55 `*_exports.h` headers found in the complete trees of these two revisions can be downloaded. They include `FUNC` registrations, constants, global and local strings, aliases, and preprocessor branches. There are **492 function/overload declarations in BennuGD classic and 616 in BennuGD2**. These are not 1,108 unique functions or Malvado functions. After downloading the sources, the [generator](../tools/catalog_api.py) reproduces the [inventory](inventory.md) and [catalog](catalog.json) offline.

Auxiliary headers and runtime reference files can also be downloaded. Each original has a pinned URL and SHA-256 in the [manifest](upstream-manifest.json). This external code is not part of the engine build.

## Limits of the full API

The snapshot covers all module export tables in the cited revisions, checked against their trees. **It is not a complete semantic specification** of DIV/Fénix/BennuGD and does not certify each build. It contains conditional functions, macros, and aliases. Language constructs, core fields, and external extensions are not fully described by these tables.

Remaining work:

1. Catalog `PROCESS`, `FRAME`, types, and structures. Games will use Lua, not a `.prg` compiler.
2. Extract core instance fields (`id`, family, priority, status), runtime variables, and constants not exported by modules.
3. Pin a Fénix version and audit original DIV functions. BennuGD alone does not prove coverage of their union.
4. For each overload, document argument names, units, ranges, defaults, errors, ownership, persistent effects, and platforms.
5. Classify equivalent, renamed, alternative, and missing functions. Check behavior against implementations and reference examples.
6. Audit third-party extensions separately. Ads and IAP require their own services and SDKs; raylib does not provide them.

## Cautions about the [historical API list](../history/legacy-api.md)

- `GET_ID` exports an `I` signature. Do not assume that `get_id()` without arguments means “my ID.” It may traverse by type; check the implementation.
- `SIGNAL` exports `II`. An entry `signal(pid)` does not describe the signal or its effects.
- `SET_FPS`, `WRITE`, `MAP_*`, colors, and sound have overloads and generation differences.
- Older Malvado uses degrees for `angle` and 1.0 for `size`. The Bennu classic inventory declares `size=100` and `PI=180000` in its angle context. Convert units deliberately.
- `write_var` with variable pointers does not translate to passing a Lua address. Propose an updatable value, Lua getter, or specific binding and explain its cost.

## Complete research for each symbol

Move a record from “inventoried” to “specified” only when it has a pinned reference, semantic signature, example, differences, and a planned conformance test. Mark it “implemented” only after code and checks exist. Calculate coverage for the pinned profile and revision. Count overloads separately and show exclusions.
