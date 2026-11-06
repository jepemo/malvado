# Malvado website foundation

This independent Astro and Starlight site is a work in progress. The production site still uses MkDocs.

## Develop and build

Install [mise](https://mise.jdx.dev/) and run:

```sh
cd docs/website
mise install
npm ci
npm run dev
```

Open `http://localhost:4321/malvado/`. Run `npm run build` to produce the static site in `dist/`, and `npm run preview` to serve that build locally. Starlight search indexes the production build; it is not available in development mode.

The pre-build and pre-development scripts generate the initial guides from existing repository documentation and examples. Edit the original files instead of generated files. The homepage also imports the existing Lua process example directly.

## Production migration

The existing `.github/workflows/pages.yml` continues to publish MkDocs and the legacy documentation. Do not deploy this incomplete site. Alpha.10 will integrate website checks into CI, audit and redirect existing public URLs, retain `/legacy/`, and replace the existing deployment only after the new site is verified. The Astro base path is `/malvado/` and the documentation lives under `/malvado/docs/`.
