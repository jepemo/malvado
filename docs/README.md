# Malvado: next generation

Migration proposal, revised on 29 October 2026. Status: **the C runtime is under development**; the CLI and GUI are planned.

The current engine uses Lua/LÖVE. The next generation uses C and raylib, with Lua as the main interface, cooperative processes, and tools to create and distribute games. Research is based on Malvado commit `24d02aba2b08f805a046aa04dcc2e29243eaa00f`.

## Recommended reading

1. [Roadmap and decisions](planning/roadmap.md).
2. [C/raylib/Lua runtime and process architecture](architecture/runtime.md) and [C core and official Lua binding](architecture/c-first.md).
3. [DIV/Fénix/BennuGD research](api/research.md).
4. [Reference module export inventory](api/inventory.md), [JSON catalog](api/catalog.json), and [source provenance](api/upstream.md).
5. [Compatibility profiles and remaining work](api/compatibility.md).
6. [Initial Lua API contract](api/lua-contract.md).
7. [Examples and acceptance criteria](examples/README.md).
8. [malvado-cli and export design](specs/cli.md).
9. [malvado-gui scope, ImGui, and DIV-inspired tools](specs/gui.md).
10. [Assets: FPG, PNG/JSON, and ZIP packages](specs/assets.md).
11. [Proposed versions](versions/README.md).
12. [Documentation for GitHub](development/publishing.md).
13. [CI, quality, and releases with GitHub Actions](development/ci.md).
14. [SDK, tools, and LuaRocks distribution](development/distribution.md).
15. [C style](development/c-style.md) and [licenses and attribution](development/licensing.md).
16. [C and Lua API documentation and examples](development/api-documentation.md).

The [historical API list](history/legacy-api.md) is for guidance. It is not a verified specification or the new API. Earlier plans remain in [history](history/README.md). Earlier examples depend on LÖVE. The [initial C runtime](versions/2.0.0-alpha.1.md) has its own example.

## Documentation layout

- `planning/`: roadmap and delivery decisions.
- `specs/`: product proposals for assets, CLI, and GUI.
- `architecture/`: runtime technical design.
- `api/`: research, proposed contract, catalog, and pinned sources.
- `versions/`: milestones and implementation status by version.
- `development/`: CI, releases, and documentation publishing.
- `examples/`: example plans and acceptance.
- `history/`: superseded documents kept for reference.
- `tools/`: documentation generators.

Each document must state whether it describes current behavior or a proposal. Put new documents in the applicable folder and link them from this index.

## Repository layout

The root contains entry points and configuration. `src/` contains the C runtime, `malvado/` contains historical Lua/LÖVE code, `examples/` contains example games, and `tests/` contains tests. CMake downloads dependency source into the build directory, which Git ignores. Group future modules by responsibility according to the [roadmap](planning/roadmap.md). Do not move code before the separation exists.

## Regenerate the reference

From the repository root, with Python 3.9 or later:

```sh
python3 docs/tools/fetch_api_sources.py
python3 docs/tools/catalog_api.py
```

The first command downloads missing pinned sources and verifies their hashes. It requires network access for downloads but no external packages. The generator runs offline and checks the hashes before regeneration. The reference API and Lua contract are separate documents. A BennuGD signature does not automatically become a valid Lua signature.
