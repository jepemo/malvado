# Malvado website

Astro and Starlight build the public Malvado 2 site. The site content comes
from the repository. `scripts/sync-content.mjs` publishes chapters from the
[site guides](../site/getting-started.md), [API guides](../api/README.md), and
[runnable examples](../../examples/README.md) into the Astro content collection. `scripts/lua-api.mjs` generates one page per Lua function
from [`docs/api/lua-globals.md`](../api/lua-globals.md) and
[`docs/api/lua-namespaces.md`](../api/lua-namespaces.md), using the family files
in [`scripts/lua-reference/`](scripts/lua-reference/). Both scripts run before builds and local
development. Edit the sources to update the generated pages.
See the [migration map](migration.md) for current routes and old fragments, and the
[Lua reference migration](lua-reference-migration.md) for family coverage.

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
to check that build locally, including search. Run
`mise exec -- npm run test:changelog` after installing Playwright Chromium
to check release navigation, migration links, mobile layout, and search.

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
   in its family file. The build reads the actual source
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

Add gallery listings to `scripts/example-gallery.mjs`. Use the same example
IDs in guides and the gallery when they share a program. For equivalent
programs, register each verified language separately and give its exact
run commands. `LanguageTabs.astro` shows every supplied version without
JavaScript and remembers the selected language when storage is available.

After building the site, run the gallery browser checks from `docs/website`.

```sh
mise exec -- npx playwright install --only-shell chromium
mise exec -- npm run test:gallery
```

## Add a Learn playground

Use `Playground.astro` to introduce a conceptual simulation. Keep its initial
diagram and explanation readable without JavaScript. Reveal native controls
only after their event handlers are ready. Use the shared styles and keep the
simulation state in its own component.

Register the component in `scripts/learn-lessons.mjs` and add the playground
marker under a Playground heading in the source lesson. Link from relevant
API entries in the family files. Check the simulation against the
current engine behavior and explain any simplifications in the lesson.

Run `mise exec -- npm run test:playgrounds` after building the site and
installing the Chromium test browser.

## Run the documentation checks

From the repository root, install the site dependencies and build the current
site. Run the shared guide and release-note checks with the reference and
Chromium tests.

```sh
mise run test-docs
mise exec -C docs/website -- npm ci
mise exec -C docs/website -- npm run build
mise exec -C docs/website -- npx playwright install --only-shell chromium
mise exec -C docs/website -- node --test tests/lua-reference.test.mjs
mise exec -C docs/website -- npm run test:gallery
mise exec -C docs/website -- npm run test:playgrounds
mise exec -C docs/website -- npm run test:changelog
mise exec -C docs/website -- python3 ../../tests/versioned_docs.py
python3 docs/tools/check_built_site.py docs/website/dist --current-only
```

The site build reads the code includes and example assets. The Lua generator
checks the global, module-only, namespace and instance-method inventories.
The runtime inventory tests check those inventories against the exported API.
LuaLS coverage checks include managed methods. Callable namespaces have
their own pages. Aliases use the shared page for their operation when the
routes match. Instance methods belong to the corresponding procedural page.
The separate legacy import and private implementation helpers are outside
the current Lua API inventory.

The checker compares supported call forms, documented parameter names and
method arity and optionality. Runtime tests cover defaults such as process
cadence. Review prose descriptions, units and other default values against
the implementation when editing a reference.

The rendered-site check covers links, fragments, assets and downloads. The
local `--current-only` option reports links that need published snapshots.
CI and Pages attach those snapshots with the same command. To run the full
check locally, use GitHub CLI access and run these commands after the other
website tests.

```sh
bash docs/tools/include_published_snapshots.sh docs/website/dist
python3 docs/tools/check_built_site.py docs/website/dist
```

The release-note tests also check publication metadata and the generated
root changelog. CI verifies tagged repository links. Historical examples
remain tied to their source revision and SDK. The example runner below uses
downloads from the current site build.

## Run the published examples

From the repository root, pass an installed CLI to the example verifier. The
headless subset works with a Lua SDK with asset support. It checks syntax and
output, and runs the Dodge! checkpoints and gameplay recipes from fresh
directories.

```sh
python3 tests/docs_examples.py /path/to/sdk/bin/malvado docs/website/dist/example-downloads --headless
```

For the full run, use Linux with Malvado's desktop build dependencies,
Xvfb, xauth, Openbox, wmctrl, xdotool and ImageMagick. Use the pinned Rust
toolchain for the published Rust projects. Build a test SDK with paced
silent audio and run the same checks as CI.

```sh
cmake -S . -B build-docs -DCMAKE_BUILD_TYPE=Debug -DMALVADO_TEST_NULL_AUDIO=ON
cmake --build build-docs --parallel
cmake --install build-docs --prefix "$PWD/build-docs/sdk"
MALVADO_SDK_PREFIX="$PWD/build-docs/sdk" CARGO_TARGET_DIR="$PWD/build-docs/rust" \
  cargo check --locked --offline --bins --manifest-path examples/rust/Cargo.toml
bash docs/tools/test_desktop_examples.sh build-docs/sdk/bin/malvado docs/website/dist/example-downloads
mise run clean-builds
```

The verifier checks every published download. It extracts bundled
assets and creates C and Rust projects with the SDK templates. Each
program must produce its expected trace or render visible content within
a time limit. The windowed check samples software-rendered pixels and
drives the input monitor's Space key. The tutorial harness provides
deterministic movement, collision and restart inputs. The normal downloads
keep their interactive controls.

Check audible playback on a host with working audio. Check physical
controllers and the Android on-screen keyboard on their supported targets.
The browser tests verify keyboard behavior, theme, mobile layout and
JavaScript errors. Review editorial quality and cross-version migration
steps with the appropriate SDK.
