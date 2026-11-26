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
