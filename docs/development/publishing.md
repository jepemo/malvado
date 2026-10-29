# Document Malvado on GitHub

See [CI and GitHub Actions](ci.md) for workflows, permissions, and release generation. See [API documentation](api-documentation.md) for the C and Lua documentation method and examples.

## Proposal

Use **Markdown in `docs/` as the source**. Review it in the same PRs as the code. Publish a static GitHub Pages site when the first runnable cut exists. Do not use GitHub Wiki as the main source: it separates documentation from engine versioning and changes.

Use MkDocs with its default theme and pin its version during implementation. Put public sources in `docs/site/`. Keep plans and historical snapshots outside the published directory. Documentation must remain readable directly on GitHub without a build. This phase does not install or publish a site.

GitHub supports [static generators through Pages workflows](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages). The future workflow must validate and build on PRs and deploy only from the configured branch or release. Adding these documents does not enable automatic deployment.

## Related references

1. **User guide:** installation, first game, processes, assets, controls, CLI, export, and troubleshooting.
2. **Lua API:** public contract records, types/ownership, examples, and availability. A reader must be able to learn game development from this guide.
3. **C API:** module reference with signatures, ownership, errors, and C examples.
4. **Compatibility:** Bennu inventory, profiles, differences, and links to pinned source.

LDoc can document Lua modules written in C, but requires explicit function annotations. It is not adopted initially. Public C headers and LuaCATS editor definitions provide language signatures. Markdown explains behavior and links to actual examples. Evaluate Doxygen or LDoc before adding them. Do not initially require a custom JSON schema. The Bennu catalog is not the Malvado API.

## Future site layout

Home → First game → Tutorials → Processes and timing → Graphics/text/input/audio → CLI and platform export → malvado-gui and process diagnostics → Assets/FPG/atlas/ZIP → Lua API → C API → Migrate from LÖVE/Malvado 1 → DIV/Bennu profiles → Development and versions.

Keep the large external inventory in the compatibility section so it does not overshadow the game creation guide. User documentation must be friendly, concise English that follows ASD-STE100 Simplified Technical English. Preserve API and symbol names.

## Examples and generation

- Show snippets from `examples/`. Link to the complete game and include controls and a small screenshot or GIF when useful.
- A stable symbol needs at least one reproducible example and a minimum version.
- CI regenerates the catalog and fails if it differs from the stored files. Check hashes, relative links, and record structure.
- Manage external overloads and implementation states separately. Inventory regeneration must not erase manual decisions.
- Add Doxygen and LuaLS when Malvado headers and contracts exist. C documentation does not replace Lua tutorials.
- Run headless examples when possible. A Markdown build does not validate a windowed game.

## Tool and asset documentation

The [GUI](../specs/gui.md) will have task-based guides, controls, screenshots, and minimum runtime/protocol versions. Distinguish tick stepping from line debugging, and temporary runtime changes from saved asset changes. The same starter-to-asteroids tutorial must work through CLI and GUI.

The [format guide](../specs/assets.md) will publish the FPG matrix by variant, import versus export, supported atlas dialects, and the meaning of a ZIP package. Do not claim FPG compatibility without limits and fixtures. UI/ImGui documentation is for development. Users need to understand their assets and conversion errors.

## Versions and project management

Keep the root README short, with a quick start, site links, and actual platform support. `docs/versions/` records proposals and acceptance. The changelog records deliveries. Publish `/dev/` and `/2.0/` when the site exists. Do not copy every document manually for each patch.

Use GitHub Issues for deliverables, with links to contracts and examples; use milestones for alphas and beta, and an optional Project for status. Issues do not replace the versioned specification. Releases include SDKs/runtimes per target, checksums, license notices, and links to that version's documentation. The current work does not create issues, tags, releases, or external publications.
