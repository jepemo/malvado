# Documentation publishing

Astro builds the Malvado 2 website from the guides in `docs/site/`.
GitHub Pages deploys it from `master` at
[`/malvado/`](https://jepemo.github.io/malvado/).

See the [website instructions](../website/README.md) for local development.
Run `mise run test-docs` to check repository links and heading anchors.

## Prepare a candidate documentation artifact

Run **Publish documentation** with both the version and the full commit SHA.
The workflow builds the guides and examples from that commit with the
current Astro renderer. Links to repository files and the source checkout
instructions use the same SHA.

The `docs-<version>-<sha>` artifact contains the site under `<version>/`
and a `candidate.json` record. See [CI and release candidates](ci.md)
for release artifact verification.

To build the same artifact locally, run these commands from the repository
root. Substitute the reviewed commit SHA and use a new output directory.

```sh
cd docs/website
mise install node
mise exec -- python3 ../tools/prepare_versioned_site.py \
  2.0.0-beta.1 FULL_COMMIT_SHA ../../build/candidate-docs
```
