# Documentation publishing

Astro builds the Malvado 2 website from the guides in `docs/site/`.
GitHub Pages deploys it from `main` at
[`malvado.dev`](https://malvado.dev/). The `docs/website/public/CNAME` file
sets the custom domain. Versioned snapshots use the same host with a
`/<version>/` path prefix.

The Pages workflow takes published snapshots from their GitHub release
bundles. The [publication catalog](../changelog/publications.json) identifies
which releases have a hosted snapshot. The Pages copies of the original
2.0.0 and 2.1.0 snapshots pin their installer commands to those releases.
The original downloadable bundles retain their historical contents.

See the [website instructions](../website/README.md) for local development.
Run `mise run test-docs` to check repository links and heading anchors.

## Prepare a candidate documentation artifact

Run **Publish documentation** with both the version and the full commit SHA.
The candidate commit must contain finalized notes for that version.
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
  VERSION FULL_COMMIT_SHA ../../build/candidate-docs --require-notes
```

## Finalize release notes

1. Update [Unreleased](../changelog/unreleased.md) with each user-visible change.
   When preparing a candidate, move its entries into `docs/changelog/VERSION.md`
   using the [release note template](release-notes-template.md). Remove
   duplicate entries, write highlights, and verify claims, upgrade examples
   and limitations on the exact candidate. Use portable Markdown. Relative
   repository links and inline images resolve at the candidate revision. Use HTTPS
   URLs for external resources.
2. Commit the finalized notes with the candidate and retain a clean
   Unreleased section for the next cycle. Keep any entries outside the
   candidate in Unreleased. If development has moved ahead after the cut,
   merge the finalized notes and remove only entries included in that
   candidate. Do not clear Unreleased during publication.
3. Leave the candidate out of the publication catalog. Preview the public
   website with the [website build](../website/README.md), and preview the
   GitHub body with `python3 tools/release_notes.py candidate VERSION SHA`.
   Run the existing [platform gate](ci.md#draft-release-bundle) and review
   the candidate documentation and bundled `RELEASE-NOTES.md`. Built docs
   carry the release body digest. The collector checks it against the body
   generated from the verified source archive.
4. With explicit maintainer authorization, verify the tag resolves to the
   tested SHA and promote the existing tested bundle. Use the bundled
   `RELEASE-NOTES.md` as the GitHub Release body. Mark alpha, beta and RC
   releases as prereleases. Add checksum or artifact links after the notes
   if needed. Keep the user-facing text and migration warnings intact.
   Use the same version, tag and revision for any separately authorized
   registry publication.

## Record publication

After the GitHub release and its assets are public, check the published
record with `gh release view vVERSION --json tagName,isDraft,isPrerelease,publishedAt,assets`.
Record the UTC date from `publishedAt` and the verified status in
`docs/changelog/publications.json`, newest date first. Use `site` for a
snapshot ready for Pages, `bundle` for docs downloadable in the release
bundle, `source` for tagged guides, or `none` where docs are unavailable.
For a newly published release, the entry on the publishing branch must use
the canonical note file from the tag. Historical backfills predate this
workflow. The Pages build renders repository files and retrieves the bundles
for the versions listed for hosting. Update the installer availability
in [release.mjs](../website/scripts/release.mjs) when the corresponding
installer and assets have been verified.

Run `python3 tools/release_notes.py index`, `mise run test-docs` and
`python3 tools/release_notes.py check --verify-tags`. After the Pages
deployment, verify the changelog link, the versioned docs and downloads.

## Recover an interrupted publication

Check GitHub's release state, the tag SHA and uploaded asset checksums
before resuming. Keep a draft or failed candidate out of the publication
catalog. If the release is public but assets are missing, finish uploading
the original verified files before recording the release. If GitHub and
its assets are already public and Pages failed, retry Pages with the same
catalog entry and verified bundle. Record actual availability separately
for any registry whose publication has not finished. Leave the Unreleased
entries from later commits in place. A candidate with changed source or
notes needs a new verified revision. Historical release description edits
require separate maintainer authorization.
