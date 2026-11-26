# Documentation navigation migration

The website leads from installation and a first program to gameplay guides,
examples, tools, language references, and contributor material. Existing
source guides and runnable programs still supply the published content.

## Split pages

The four old URLs serve static link pages. Each old heading links to its
destination. A bookmarked fragment scrolls to that heading even with
JavaScript disabled. Search indexes the destination pages.

| Source and old route | Old section | Current destination |
| --- | --- | --- |
| [Getting started](../site/getting-started.md), /getting-started/ | Install the SDK and Uninstall the SDK | /docs/start/installation/#install-the-sdk and #uninstall-the-sdk |
| Same | Create and run a game and Create a project in older snapshots | /docs/start/first-program/ |
| Same | Lua in VS Code | /docs/tools/editor/#lua-in-vs-code |
| Same | Build from source (contributors) and Prepare your computer and Run Hello World in older snapshots | /docs/contributing/build/ |
| Same | Try more examples | /docs/examples/#basics for lessons and /docs/contributing/testing/ for test commands |
| [SDK guide](../site/sdk.md), /sdk/ | Install a local development build and Windows SDK prefix | /docs/contributing/build/ with Windows player requirements at /docs/tools/platforms/#windows-players |
| Same | Build and install the SDK | /docs/contributing/release/#build-and-install-the-sdk for packaging and /docs/start/installation/#install-from-an-archive for verification and setup |
| Same | Create and run a game and Create a Rust game | /docs/start/project-structure/#create-and-run-a-game and #create-a-rust-game |
| Same | Export a game and Export for Android | /docs/tools/export/#export-a-game and #export-for-android |
| [Gameplay guide](../site/api.md), /api/ | Run and stop processes | /docs/learn/process-model/#run-and-stop-processes |
| Same | Organize a level | /docs/patterns/level-lifecycle/#organize-a-level |
| Same | Choose a reference | /docs/reference/#choose-a-reference |
| Same | Keep resources alive | /docs/learn/resources/#keep-resources-alive |
| Same | Choose a runtime and target | /docs/tools/platforms/#choose-a-runtime-and-target |
| [Catch! sources](../../examples/catch/README.md), /docs/samples/ | Lua and C and Rust (experimental) and Catch! | /docs/examples/catch/#lua, #c, #rust-experimental and #catch |
| [First Rust games](../../examples/rust/README.md), /docs/samples/ | Hello World and Moving balls | /docs/examples/rust-basics/#hello-world and #moving-balls |

[The section map](scripts/guide-migration.mjs) records the heading destinations
used by the generator. The three source guides remain at their GitHub paths.
The candidate builder can still read guides from the selected Git revision.
Published release snapshots keep their existing HTML.

## Retained routes

| Existing route | Source | Current navigation |
| --- | --- | --- |
| / | [Landing page](src/pages/index.astro) and Lua sources in examples/helloworld, examples/meteors, and examples/move | Landing page with updated documentation links |
| /docs/ | [Introduction](src/content/docs/docs/index.md) | Getting started |
| /docs/learn/process-model/ | [Thinking in processes](../site/learn/process-model.md) | Learn Malvado, followed by [lifecycle](../site/learn/process-lifecycle.md), [timing](../site/learn/frames-and-time.md), and [communication](../site/learn/process-communication.md) |
| /docs/concepts/lua-processes/, drawing/, keyboard/, images/, animations/, text/, cameras/, collision/, audio/ | Same-named [API guides](../api/README.md) | Learn Malvado |
| /docs/concepts/lua-language-server/ | [LuaLS guide](../api/lua-language-server.md) | Linked from Tools and deployment → Editor setup |
| /docs/concepts/lua-instance-methods/ | [Instance methods](../api/lua-instance-methods.md) | Lua API |
| /docs/concepts/c-contract/ | [C process API](../api/c-contract.md) | C API |
| /docs/concepts/rust-processes/, rust-window/, rust-drawing/, rust-input/, rust-atlas/, rust-fpg/, rust-camera-animation/, rust-font/, rust-collision/, rust-audio/ | Same-named [Rust guides](../api/README.md) | Rust API (experimental). Titles include the label in search results. |
| /docs/concepts/process-cleanup/, runner-timing/, resource-lifetime/ | Same-named [technical contracts](../api/README.md) | Contributing → Technical contracts. Gameplay guarantees stay linked from Learn and API. |
| /docs/reference/lua/, /docs/reference/lua/{family}/, /docs/reference/lua/{family}/{function}/ | [Lua exports](../api/lua-globals.md), [Lua namespaces](../api/lua-namespaces.md), [generator](scripts/lua-api.mjs), and [family content](scripts/lua-reference/) | Lua API. Families include processes, input, graphics, drawing, images, animations, text, audio, cameras, collision, world, FPG, platform, and module. Each family links its functions. |

Retained guide headings stay at their existing fragments. Repository-only
material stays in the [API index](../api/README.md),
[development guide](../development/README.md), [example catalog](../../examples/README.md),
[version plan](../versions/README.md), and [Roadmap](../roadmap/README.md).
The sidebar links directly to the C reference, Rust support policy,
engine architecture, binding ownership, and documentation guidelines.
The [website CLI reference](scripts/guide-migration.mjs) summarizes the
installed commands and links to the [full CLI contract](../development/cli.md).
The platform and release pages link to the detailed Android, SDK, and
publishing guides. The searchable example gallery groups runnable programs
by topic and difficulty. The Catch! language tabs keep the old sample
fragments. Example pages include source, downloads and run instructions.

See the [documentation roadmap](../roadmap/README.md#documentation) for the
next editorial phases.
