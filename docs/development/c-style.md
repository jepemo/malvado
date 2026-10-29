# C code style

Write code that is easy to understand, change, and debug. Prefer clear code and explicit contracts to unnecessary abstractions or language tricks.

- Give each small function one clear task. Use specific names that are consistent with their scope. Keep data with the same lifetime together.
- Put one important operation on each line. Avoid compressed functions, complex macros, and unnecessary nesting. Return early on errors.
- Document invariants, ownership, and decisions that are not obvious. Keep comments consistent with the code.
- Make creation, destruction, states, and errors explicit. Check sizes, limits, and allocation failures. Do not hide global effects behind unclear names.
- Start each Malvado C source and header file with the full ASCII logo header, attribution, and `SPDX-License-Identifier: Apache-2.0`, as in `src/main.c`. See [licensing](licensing.md). Keep third-party notices unchanged.
- Keep header interfaces small. Put dependencies and internal details in `.c` files. Separate portable code from platform APIs when the separation has a clear purpose.
- Follow the local format: four-space indentation, clear braces, reasonable line lengths, and blank lines between functions. Do not reformat third-party code in `vendor/`.
- Test observable behavior, especially errors, boundary conditions, and object lifetimes. Do not duplicate the implementation in tests.

This guide applies to new contributions and gradual refactoring. It does not claim that all existing code follows these rules.
