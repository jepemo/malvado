# Write and maintain Malvado documentation

Help readers understand what the engine does so they can predict what
happens when they change their game. Open with a concrete problem, a question,
or a result the reader can see. Use the [writing templates](documentation-templates.md)
to plan a new page or revise an existing one.

## Choose the reader's task

| Page | Reader's task |
| --- | --- |
| Concept chapter | Understand how Malvado works and predict a result. |
| Tutorial | Learn by building something in small, working steps. |
| Pattern or recipe | Solve a recurring game problem and understand when to use the solution. |
| Gallery entry | Find a runnable example by topic and see what it does. |
| API reference | Look up the exact public contract for an operation or value. |
| Internal contract | Maintain the engine or bindings, including invariants and implementation boundaries. |

Use the [site migration map](../website/migration.md) for current routes.
Link related pages where a reader needs more detail. Put public guarantees
in the API guides and reference. Keep scheduler internals and binding
implementation rules in contributor material.

## Show how it works

- Teach one concept at a time. Start with the smallest working program that
  shows the result. Explain the engine behavior that produces it.
- Give readers a mental model they can test. Address a likely mistake when
  it changes what they expect. Offer one small edit and say what to observe.
- Use a diagram when it explains timing, ownership, or space more clearly
  than prose. Label the states, axes, or units readers need to understand it.
  Describe the main result in text. Label a browser simulation as a
  conceptual model and explain what it demonstrates.
- Write short, direct sentences. Define unfamiliar terms at first use.
  Remove generic introductions, repetitive boilerplate, and marketing claims.
- Teach in Lua first. Include C and Rust where they help the reader, with
  idiomatic code and verified behavior. State language or platform limits.

Use these sources for teaching ideas. Write Malvado's own examples, prose,
and visuals.

- Expose a surprising behavior through a small experiment.
  [Julia Evans and Wizard Zines](https://wizardzines.com/)
- Build confidence with a series of steps that each have a clear result.
  [Godot's step-by-step guide](https://docs.godotengine.org/en/stable/getting_started/step_by_step/index.html)
- Explain the game problem before introducing a pattern.
  [Game Programming Patterns](https://gameprogrammingpatterns.com/)
- Make the effect of changing a value visible.
  [The Book of Shaders](https://thebookofshaders.com/)
- Match the form of a page to what the reader wants to do.
  [Diátaxis](https://diataxis.fr/)

## Use runnable examples

Use current public APIs, named constants where available, and documented
defaults. Keep setup small. Use built-in resources when they teach the same
idea. Supply any required assets with their licenses and attribution.
Give the entry point, requirements, working directory, run command, expected
result, and how to stop. Keep paths independent of the repository root.

Use one source for each program wherever possible. Link or include the
complete program and its required files when you show an excerpt. Each Lua
function reference needs a complete runnable example or a direct link to a
complete variant that demonstrates the function. Related functions can
share a program.

Keep published programs under [examples](../../examples/README.md). Use the
[website include and download conventions](../website/README.md#publish-a-runnable-example)
to reuse the same source in a reference, Learn chapter, tutorial, or gallery
entry. For each tutorial checkpoint, give the reader a complete program and
all files needed to resume from that step.

Run the example with the documented command from a clean working directory.
Check its output or behavior and bound the run when verifying it. Use a
graphics or audio backend when the expected result depends on it. Report
specific platform or setup limits. Compile and check published C and Rust
variants where applicable.

## Maintain the public API reference

Public C headers, the Lua export inventories and Rust crate sources define
names and types. The [API guides](../api/README.md) describe behavior,
ownership, errors and platform limits. Follow the
[public API documentation rules](../../AGENTS.md#public-api-documentation)
when a public API changes.

For website Lua references, edit the appropriate family file in
[`docs/website/scripts/lua-reference/`](../website/scripts/lua-reference/). The generator uses the
[global](../api/lua-globals.md) and [namespace](../api/lua-namespaces.md)
inventories for page structure. Check the annotations in
[the LuaLS definitions](../../types/) and the
[instance methods reference](../api/lua-instance-methods.md) for operations
with a managed receiver. Verify signatures and behavior against the
implementation and tests. Use the
[function template](documentation-templates.md#function-reference)
and see the [`rand` reference](https://malvado.dev/docs/reference/lua/platform/rand/)
for a short example of the format.

## Catalog and link checks

Run `mise run catalog` to fetch pinned upstream sources, verify their hashes
and regenerate the reference inventory. Keep this catalog separate from
the Malvado API.

Run `mise run test-docs` to verify local links, heading anchors and GitHub
links to current repository files. Build the
[website](../website/README.md) and check the affected pages, including
generated links and code. The [publishing guide](publishing.md) covers
versioned snapshots. The
[documentation roadmap](../roadmap/README.md#documentation) tracks broader
example coverage and CI enforcement.

## Executable Rust guide checks

`tools/test_rust_docs.py` reads every Rust block in `docs/api/*.md`, builds each
as a main-thread executable against installed `malvado` crate sources, then runs
it with a bounded timeout. It uses the actual guide text, including Rustdoc
hidden setup lines. No ignored or compile-only Rust blocks are accepted. The
fixture supplies existing licensed PNG, font, FPG, WAV and OGG inputs in a
temporary directory; no teaching example or game asset catalog is added.

`mise run test-rust-graphics` includes these checks for both SDK profiles and
hosts with a display. For an existing graphical SDK, run:

```sh
mise exec -- python3 tools/test_rust_docs.py --sdk "/path/to/installed sdk"
mise exec -- python3 tools/test_rust_docs.py --sdk "/path/to/installed sdk" --release
```
