# Malvado website

Astro and Starlight build the public Malvado 2 site. The site content comes
from the repository: `scripts/sync-content.mjs` copies the guides from
[`docs/site/`](../site/getting-started.md), the aligned image example from
[`examples/features/image/`](../../../examples/features/image/README.md),
and the concept guides from [`docs/api/`](../api/README.md) into the Astro
content collection. `scripts/lua-api.mjs` generates one page per Lua function
from [`docs/api/lua-globals.md`](../api/lua-globals.md) and
[`docs/api/lua-namespaces.md`](../api/lua-namespaces.md), using the per-function
text in `scripts/lua-api-content.mjs`. Both scripts run before builds and local
development. Edit those sources; the copied files are generated.

From the repository root, run these commands with mise installed.

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
