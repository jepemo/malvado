# C and Lua API documentation

Status: an initial MkDocs site is available. Full references and publication remain planned. Research date: 2026-10-29. Both APIs are first-class and need references, runnable examples, and getting-started guides. Document the [C core and official Lua binding](../architecture/c-first.md) with the same behavior and their language-specific differences.

## Recommendation: Markdown + MkDocs + GitHub Pages

1. **Markdown in Git** for documentation readable on GitHub and reviewed with code.
2. **MkDocs with the default theme**, pinned in documentation dependencies, for navigation, search, and one published site. MkDocs uses Markdown and `mkdocs.yml`; no custom frontend is needed. See the [official documentation](https://www.mkdocs.org/user-guide/writing-your-docs/).
3. **GitHub Actions and Pages** to build on each PR and deploy from an authorized branch or tag. [GitHub Pages](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages) supports static-generator workflows.
4. **API sources:** declarations and comments in public C headers; annotated Lua definitions for the C-registered binding; Markdown module pages with contracts and examples in both languages.

This option uses one site generator. It does not initially require a custom schema, C documentation plugins, or separate generated HTML for each language. Important content remains readable on GitHub without MkDocs. Add generation later if the number of functions makes it useful.

## Tool comparison

| Option | Benefit | Cost for Malvado | Decision |
| --- | --- | --- | --- |
| Markdown + MkDocs | One site, links, search, PR review | Records and signatures require disciplined maintenance | Initial base |
| Doxygen | Extracts C symbols and comments; supports Markdown and HTML/XML | More generation, possible duplication, and integration to maintain | Use compatible header comments; evaluate generation as the C API grows |
| LDoc | Generates Lua documentation and can process Lua modules written in C | Requires explicit annotations for functions registered in C and another tool/output | Evaluate later; not required for first publication |
| Lua Language Server / LuaCATS | Editor types, signatures, and help from annotations | A C module needs maintained or generated Lua definitions | Use for editor support, not as a required site generator |

Sources: [Doxygen C comments](https://www.doxygen.nl/manual/docblocks.html), [LDoc](https://stevedonovan.github.io/ldoc/manual/doc.md.html), and [LuaCATS](https://luals.github.io/wiki/annotations/). LDoc can document Lua modules written in C, but cannot automatically infer their function names; document them explicitly. LuaCATS is the Lua Language Server format. Do not claim all versions are identical to EmmyLua.

## Proposed layout

```text
README.md                         short entry point, installation, links
mkdocs.yml                        site configuration
requirements-docs.txt             pinned MkDocs version
include/malvado/*.h               C API declarations and comments
bindings/lua/definitions/         annotated .lua definitions for tooling only
examples/c/                       actual C projects and examples
examples/lua/                     actual Lua projects and examples
tests/                            behavior and example tests
docs/site/index.md                public site home
docs/site/getting-started/        first C and Lua games
docs/site/api/c/                  C reference by module
docs/site/api/lua/                Lua reference by module
docs/site/guides/                 shared tutorials and recipes
docs/site/migration/              migration and compatibility
```

`docs/site/` contains the public site sources. The site output is in `build/site/`. This prevents MkDocs from copying `docs/api/upstream/` and large catalogs. Plans, historical sources, and analysis stay in their existing `docs/` folders and can be linked on GitHub. The proposed reference directories do not exist yet.

## Source of truth by language

**C:** signatures and types live in installable public headers under `include/malvado/`. Document each declaration's purpose, parameters, units, returns/errors, ownership, lifetime, and version. Use `/** ... */` comments and tags such as `@param` and `@return`, compatible with Doxygen, even if the initial site is written in Markdown. Check reference names and arguments against the headers. Do not publish a future API as if it exists.

**Lua:** public signatures, types, and comments live in a LuaCATS-annotated `.lua` definitions file for the C binding. This file is for **types and tooling**. Do not install or load it as an implementation that replaces `require("malvado")`. Each function's visible contract is also explained on its Markdown page and links to its example. Update binding registration, definitions, and documentation together. CI checks exported names, and tests exercise signatures and behavior. The Bennu reference inventory is not part of this contract.

C headers and Lua definitions provide language signatures. Markdown pages provide narrative behavior, examples, and language differences. Keep header and definition comments concise and link to the module. Matching names does not prove that types, returns, or semantics are correct; review and test them. Avoid a required third JSON contract until it demonstrably generates and validates both outputs with less maintenance. Do not generate documentation from a nonexistent Lua facade.

## Minimum API record

For each public function or type and each available variant, include:

- Name, exact language signature, module, minimum version, and actual status: implemented or planned.
- Purpose, arguments, types, units, ranges, optional values, return, errors, and platform.
- In C: ownership, nullability, pointer/handle lifetime, and destructor. In Lua: types, tables/userdata, errors, and reference management.
- Process/tick effects, related examples, and links between C and Lua APIs when applicable.
- A link to a **runnable example** with commands, result, and required resources. One example can cover multiple functions; a separate program per symbol is not required.

Organize by module: context, processes, signals/messages, time, window, graphics, text, input, and audio. Keep separate C and Lua indexes. Put shared concepts in one guide and link both references. Document language-specific details in their own references.

## Examples as source

Keep examples in `examples/`, not as separately maintained blocks in every page. Each folder needs a README, requirements, build/run steps, and expected output. C examples include `main.c` and build configuration. Lua examples include `main.lua` and configuration when a CLI exists. `hello`, processes/signals/messages, and a small game need C and Lua versions that show the same behavior. Add a mixed example and document message type limits.

On the initial site, link to the actual GitHub file and example README. Release documentation links to the **version tag**, not `main`. Development docs may link to `main`. If snippets are included later, extract regions automatically from the actual file with a pinned dependency. Basic MkDocs does not include a generic file-insertion directive. Do not duplicate examples that can silently diverge.

Headless examples must terminate automatically in CI. Graphics examples can use smoke mode and visual checks on an appropriate runner. Compile at least one C example against **installed** headers and library with Lua disabled. Run Lua examples with the official runner. Building the site does not replace running the examples.

## Build, CI, and publication

To build or preview the site:

```sh
python3 -m pip install -r requirements-docs.txt
python3 -m mkdocs build --strict
python3 -m mkdocs serve
```

CI builds the site in strict mode. The first page links to the existing guides on GitHub. Signature checks and complete C and Lua references remain planned. A site build does not test external links or replace example tests.

The Pages workflow deploys from `master` to the publication environment. PRs build but do not deploy. The initial site is at the root, and the previous documentation is under `/legacy/`. When versioning exists, put development docs at `/dev/` and stable docs by minor at `/2.0/`. Identify commit/version and link the relevant examples. Define retention before replacing versions.

## Milestones and criteria

| Milestone | Delivery | Acceptance |
| --- | --- | --- |
| Alpha.2, C refactor | Actual C and Lua context/process contracts; minimal equivalent examples | C game without Lua, Lua game, and mixed cases run; review records and errors with API |
| Alpha.3 | Graphics, input, text, and audio in both references; initial MkDocs site | Strict build and links; examples work on supported targets |
| Alpha.4 | C SDK, Lua runner, CLI, and export guides | Create/build/export one C and one Lua game on verified paths |
| Beta | Complete public-symbol inventory and ownership records | Each stable symbol has documentation, runnable cases, behavior test, and parity review |
| Release 2.0 | Versioned site and pinned sources | Documentation, examples, and binaries point to the same version; Pages published after configuration |

Add Doxygen or LDoc only after a small trial verifies signature coverage, examples, and maintenance cost. Their output does not automatically replace narrative documentation. If generation is selected, use one source for each datum and fail CI for stale output or signatures.
