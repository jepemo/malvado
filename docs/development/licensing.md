# Licenses and attribution

Jeremías Pérez created Malvado. The license selected for **new Malvado 2 code** is Apache License 2.0. The license text is in [LICENSE](../../LICENSE). New Malvado C files, examples, and tests identify their authors and include `SPDX-License-Identifier: Apache-2.0`. This choice does not change the licenses of dependencies or historical files.

## Historical Malvado 1 code

The 1.0 rockspec declares Apache 2.0. Several historical Lua files state GPL version 3 or later. Keep their headers and the historical rockspec while ownership and distribution terms are verified. The Apache license for new code does not automatically change permissions already granted for distributed copies. Before you reuse or redistribute historical code under Apache, confirm ownership and document the decision. Third-party contributions require permission from their rights holders if their terms do not permit the change.

## Dependencies and references

Lua downloaded during the build retains its MIT license. Raylib and its dependencies retain their own licenses. The BennuGD files in `docs/api/upstream/` are reference material and retain their original notices. Do not add Apache headers to external files or remove their notices. Distributions of the runtime, SDK, and games must include the notices and license texts required by the dependencies they contain.

Before each release, review the origin and license of all assets, historical examples, and new dependencies. Distributions and any future 2.0 rockspec must state the scope of Apache 2.0 and include the applicable third-party notices.
