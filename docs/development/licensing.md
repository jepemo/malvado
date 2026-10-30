# Licenses and attribution

Jeremías Pérez created Malvado and has selected Apache License 2.0 for the Malvado 2 rewrite. The license text is in [LICENSE](../../LICENSE). New Malvado C files, examples, and tests identify their authors and include `SPDX-License-Identifier: Apache-2.0`. This choice does not change third-party licenses or the terms of copies previously distributed.

## Historical Malvado 1 code

The old Lua/LÖVE engine had GPL-3.0-or-later notices while its 1.0 rockspec declared Apache 2.0. The owner has chosen the Apache-2.0 rewrite and removed that engine and rockspec from the current tree. The [historical revision](https://github.com/jepemo/malvado/tree/4670d8a) remains available with its original notices. The old examples remain as migration references; they are not yet Malvado 2 examples. This decision does not revoke the terms of earlier copies or relicense third-party material.

## Dependencies and references

Lua and cJSON downloaded during the build retain their MIT licenses. cJSON is copyright Dave Gamble and cJSON contributors; its license is installed with the assets library. Raylib and its dependencies retain their own licenses. The BennuGD files in `docs/api/upstream/` are reference material and retain their original notices. Do not add Apache headers to external files or remove their notices. Distributions of the runtime, SDK, and games must include the notices and license texts required by the dependencies they contain.

Before each release, review the origin and license of all assets, historical examples, and new dependencies. Distributions and any future 2.0 rockspec must state the scope of Apache 2.0 and include the applicable third-party notices.
