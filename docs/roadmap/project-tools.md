# Project tools

Status: future proposal. The [CLI reference](../development/cli.md)
describes current project discovery, builds and exports.

Review these ideas when projects need them:

- A project manifest for explicit entry points, metadata, asset selection
  and custom window defaults. Define precedence against language-specific
  project files and game code. Version the manifest independently.
- More project templates and a way to list them. Keep the starter small
  and use licensed shared assets in samples.
- Explicit target SDK discovery and installation. Verify version, target,
  checksums and toolchain before selecting a runtime. Define host and target
  coverage for each export.
- Structured CLI results for tools such as the [graphical workshop](gui.md).
  Share validation and packaging with the existing CLI.

Start with one real project. Test invalid configuration, project and asset
paths, installed and relocated SDKs, and exports on the selected targets.
