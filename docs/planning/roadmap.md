# Malvado release direction

Status: current owner direction. The first 2.0 release focuses on the existing
engine, C/Lua/Rust alignment, CLI, automated platform validation and usable
documentation. New API families and larger tools are evaluated after release.

- [Current version plan](../versions/README.md): alpha.7 integration, alpha.8
  platform automation, beta candidate review, then 2.0 publication.
- [Future feature roadmap](../roadmap/README.md): unassigned proposals, the API
  candidate catalog, effects/shaders, spatial attachments, graphics modes,
  examples/minigames, assets, editor and website work.
- [Delivered API selection](../api/family-review.md) and
  [current C symbols](../api/current-c-api.md): existing scope and evidence.
- [Architecture](../architecture/multi-language.md), [CLI](../specs/cli.md), and
  [distribution](../development/distribution.md): shared engine and delivery contracts.

The historical LÖVE migration plan and earlier numbered post-2.0 assignments
remain in Git history. Existing references to this path continue to resolve.
Future proposals are not implementation specifications. Breaking changes within
2.x remain possible until the owner explicitly changes that policy; document
and verify each approved change, including native ABI/SDK incompatibilities.
