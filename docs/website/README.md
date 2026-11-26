# Malvado website

Astro and Starlight build the public Malvado 2 site. The site content comes
from the repository. `scripts/sync-content.mjs` publishes chapters from the
[site guides](../site/getting-started.md), [API guides](../api/README.md), and
[runnable examples](../../examples/README.md) into the Astro content collection. `scripts/lua-api.mjs` generates one page per Lua function
from [`docs/api/lua-globals.md`](../api/lua-globals.md) and
[`docs/api/lua-namespaces.md`](../api/lua-namespaces.md), using the per-function
text in `scripts/lua-api-content.mjs`. Both scripts run before builds and local
development. Edit the sources to update the generated pages.
See the [migration map](migration.md) for current routes and old fragments.

The example generator uses Python 3's standard library to create ZIP
downloads. From the repository root, run these commands with mise installed.

```sh
cd docs/website
mise install node
mise exec -- npm ci
mise exec -- npm run dev
```

Open `http://localhost:4321/`. Use `mise exec -- npm run build`
to build the static site in `dist/`. Use `mise exec -- npm run preview`
to check that build locally, including search.

See the [publishing guide](../development/publishing.md) for GitHub Pages
and versioned documentation.

## Publish a runnable example

1. Use the [documentation templates](../development/documentation-templates.md).
   Put the complete program under [examples](../../examples/README.md)
   and use built-in graphics and fonts where they suit the lesson.
2. Register the example in `scripts/examples.mjs`. Give it a unique ID, a
   repository directory and the source path within that directory. For an
   example with assets, list each required file and its path in the ZIP.
   Supply credit and any license notices needed to redistribute the files.
   Reuse shared assets when possible. The [image lesson](../../examples/features/image/README.md)
   uses the same tile in C, Lua and Rust.
3. Use `publishExample(id, route)` when generating a guide, tutorial, or
   gallery page. Insert its `links` and `snippet` in the page. For the
   Lua reference, set `exampleId`, `exampleSetup`, and `exampleResult`
   in `scripts/lua-api-content.mjs`. The build reads the actual source
   and fails if a declared file is missing. Use the same ID across pages
   or link to the page that introduces the lesson.
4. Give each tutorial checkpoint its own ID and complete source tree.
   List only the assets that checkpoint needs. Treat each checkpoint as a
   separate runnable download. When adding a source directory, also list
   it in `docs/tools/prepare_versioned_site.py` so candidate builds use
   files from the selected revision.

Test the downloaded file or extracted ZIP from a fresh directory outside
the repository. Use the exact command in the page and check the documented
result. Keep Lua asset paths relative to the entry script's directory and
preserve that layout in the ZIP. Use a matching SDK and state when the
example needs a display, audio device, or other platform setup. For C and
Rust, link the checked-in source and give the SDK project and build commands.

Use `malvado run --headless` for Lua examples that need no graphics or audio
and end on their own. For graphical examples, use a desktop display or the
virtual display already used by CI. Use a timeout and check output or
observable behavior. Keep the normal exit controls for interactive examples.
Use the existing [lesson verifier](../../tools/test_beta_examples.py) and
[CI tasks](../development/ci.md) when adding automated checks.
