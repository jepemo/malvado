# Malvado Contribution Instructions

## 1. Project principles

Malvado is a game engine inspired by DIV Games Studio. It uses a C core and provides C and Lua APIs.

Follow these principles:

- **Simplicity first.** Make game development easy. Prefer small, clear APIs over complex abstractions.
- **Developer experience matters.** Keep common tasks short and intuitive. Prefer useful defaults and explicit behavior.
- **One engine, two languages.** C and Lua are first-class APIs. Both use the same core behavior and process scheduler.
- **Explicit behavior.** Define ownership, lifetime, errors, and side effects. Avoid hidden state and surprising behavior.
- **Maintainability over cleverness.** Prefer code that is easy to read, test, debug, and extend.
- **No speculative complexity.** Do not introduce features, dependencies, or abstractions for hypothetical future requirements.

Preserve the process-based programming model that defines Malvado. Do not introduce a second scheduler or a different process model without an explicit design decision.

## 2. Architecture and API design

- Keep the core usable without Lua or a graphics backend.
- Keep platform-specific code separate from the core when this has a clear purpose.
- Avoid duplicate implementations of the same behavior in C and Lua.
- Follow the existing architecture and documented contracts.
- Prefer composition and small, focused components over large, tightly coupled subsystems.
- Do not expose internal implementation details through public APIs unless necessary.

For public API changes:

- Choose clear and consistent names.
- Define parameters, return values, errors, and edge cases.
- Define ownership and cleanup rules when applicable.
- Consider C and Lua behavior together.
- Update relevant documentation, examples, and tests.
- Preserve established behavior unless the change explicitly revises its contract.

Treat approved specifications as contracts. Treat experimental designs and proposals as non-binding until a decision is recorded.

When an architectural decision is unresolved, explain the trade-offs before introducing a public contract or a difficult-to-reverse design.

### Compatibility policy

Malvado 2.0 is not backward compatible with Malvado 1.0.

- Do not preserve old APIs, compatibility shims, or behavior only for 1.0 compatibility.
- Reuse historical Lua names or behavior when they improve the 2.0 API.
- Treat the C API as a new API.
- Treat earlier 2.0 alpha APIs as experimental. They can change before the beta API freeze.
- Evaluate interoperability with external formats separately from API compatibility.

## 3. Code conventions

- Follow the [C style guide](docs/development/c-style.md).
- Follow established conventions in surrounding Lua and other project files.
- Use descriptive names and small functions with clear responsibilities.
- Keep changes focused. Avoid unrelated refactoring and formatting changes.
- Avoid unnecessary dependencies and abstractions.
- Do not modify third-party source code or notices unless the task explicitly requires it.

### License headers

Start each Malvado-owned C source and header file with the complete ASCII logo header, attribution, and SPDX license identifier, as in `src/main.c`.

Preserve third-party licenses, notices, and attribution.

## 4. Documentation and examples

Write documentation, README files, examples, and code comments in English.

Follow ASD-STE100 Simplified Technical English principles:

- Use short, direct sentences.
- Give each sentence a clear purpose.
- Use consistent technical terms.
- Avoid unnecessary jargon and ambiguous language.
- Preserve code identifiers, proper names, and quoted source material.

Make user-facing documentation friendly, welcoming, and easy to follow.

- Explain what a feature does and when to use it.
- Prefer practical, runnable examples over long explanations.
- Show the simplest useful example first.
- Explain important limitations and error behavior.
- Keep documentation concise without omitting useful information.

Use the original root `README.md` as a reference for its approachable, example-led style.

Keep documentation and examples consistent with the implemented API. Do not present planned features as available.

## 5. Development workflow

Before making changes:

1. Read the relevant implementation, tests, and documentation.
2. Identify the affected contracts and components.
3. Keep the implementation within the requested scope.
4. Prefer existing project patterns when they satisfy the requirement.

During implementation:

- Make focused changes that are easy to review.
- Add or update tests for changed behavior.
- Update documentation when user-visible behavior changes.
- Avoid unrelated cleanup and speculative improvements.
- Do not silently change public contracts or expand the milestone scope.
- Record important design decisions and their reasons in the appropriate documentation.

Use the following project references when relevant:

- [Version plan](docs/versions/README.md)
- [Architecture and roadmap](docs/planning/roadmap.md)
- [C style guide](docs/development/c-style.md)
- [Development commands](mise.toml)

Prefer existing specifications over assumptions. If documentation and implementation disagree, identify the difference instead of silently choosing one.

## 6. Testing and verification

Every implementation change must have appropriate verification.

Use the relevant existing commands:

| Command                  | Purpose                             |
| ------------------------ | ----------------------------------- |
| `mise run test`          | Standard build and automated tests  |
| `mise run test-c`        | C core without Lua or raylib        |
| `mise run test-headless` | Lua runtime without graphics        |
| `mise run test-c-assets` | C asset tests without Lua or raylib |
| `mise run test-visual`   | Automated graphics checks           |

Choose tests based on the affected components. Run broader tests when a change affects shared behavior.

- Test observable behavior, not only successful compilation.
- Include invalid inputs, failure paths, and boundary conditions when relevant.
- Test resource ownership, cleanup, and process lifetime when affected.
- Verify C and Lua behavior when a feature is shared.
- Add automated graphics, input, and emulator checks in the relevant alpha milestone.
- Keep manual user-flow and physical-device testing in beta.
- Do not replace required automated tests with planned manual tests.

Do not claim a test passed unless it ran successfully.

Report which checks passed, failed, or could not run, and explain relevant verification limits.

## 7. Versioning and scope management

Each version must have a clear outcome and limited scope.

- Define each checklist item as a specific deliverable with acceptance criteria and required automated tests.
- Complete the full scope of an item before marking it as done.
- Record partial progress in notes or commit history.
- Add checklist items during implementation only for specific gaps.
- Do not expand a version with unrelated work.
- Assign deferred work to a named version and explain why it is outside the current scope.
- Keep incomplete work open until its implementation and required verification are complete.

Before declaring a version complete:

1. Review its outcome, scope, dependencies, and open items.
2. Verify the implementation and required automated tests.
3. Check the documentation and version identifier.
4. Verify CI on the exact commit selected for the tag.
5. Record any remaining limitations accurately.

Do not equate a completed checklist, source tag, and published release. Each has its own requirements.

Do not create tags or publish releases without an explicit request.

## 8. Final delivery

When completing a development task, provide a concise summary of:

- What changed and why.
- Which tests were run and their results.
- Any remaining limitations or incomplete work.
- Any decisions or follow-up work that still need attention.

Do not report incomplete functionality as complete. Keep the final report short, accurate, and useful.
