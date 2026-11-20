# Malvado website

Astro and Starlight build the public Malvado 2 site. Edit the guides in
[`docs/site/`](../site/getting-started.md) and the runnable examples in
[`examples/`](../../examples/README.md). Astro copies them into its content
collection before builds and local development.

From the repository root, run these commands with mise installed.

```sh
cd docs/website
mise install node
mise exec -- npm ci
mise exec -- npm run dev
```

Open `http://localhost:4321/malvado/`. Use `mise exec -- npm run build`
to build the static site in `dist/`. Use `mise exec -- npm run preview`
to check that build locally, including search.

See the [publishing guide](../development/publishing.md) for GitHub Pages
and versioned documentation.
