# Graphics modes 7, 8, and 9

Status: design proposal. These modes are not implemented. Their MVPs are planned for alphas before the 2.0 release. See the [roadmap](../planning/roadmap.md). This proposal does not promise compatibility with DIV, Bennu, or the Mode9 DLL.

## Purpose

Add optional perspective graphics while keeping the current 2D engine and the shared process scheduler. Mode 7 draws a flat perspective floor. Mode 9 draws 3D geometry and sprites in the same scene. Mode 8 uses sectors to describe connected rooms and can reuse the Mode 9 renderer. Retro and HD-2D are visual styles, not separate engines.

Keep the first version of each mode small. Add C and Lua access together. Do not add Lua or raylib dependencies to the core. Do not create another scheduler or an ECS.

## Current starting point

- `MvContext` owns scheduling and time. It does not currently own a graphics scene.
- `platform.h` exposes backend-neutral image handles, image regions, transforms, windows, and immediate 2D drawing. The raylib backend uses one window on the main thread. Image resources must be destroyed before the window closes.
- The Lua binding currently builds and sorts the 2D process draw queue. It draws persistent text and fades after process drawings. C callers control their own drawing order. There is no shared C scene or 3D draw queue yet.
- The headless backend does not open a window or load GPU images. It is not a simulated 3D renderer.
- Atlas animation metadata and frame selection, input, the clock, and process lifecycle can be reused. They do not by themselves provide 3D animation, collision, or camera behavior.

## Architecture and contracts to define

Introduce backend-independent scene data and operations. Keep the renderer and GPU resources behind a backend interface. Do not expose raylib types. A scene uses the existing scheduler; it does not execute processes. Resolve how a scene relates to `MvContext` and to the existing single window before implementation. Do not assume that current graphics resources already belong to a context.

Use an explicit 3D position and transform for scene objects. Preserve 2D process `x` and `y` in pixels and `z` as draw order. A process can update or own a scene object; it must not silently reinterpret its 2D fields. Specify ownership and cleanup when the process ends, when a scene is destroyed, and when a window closes. Shared images and models must remain valid while referenced. Define invalid-handle errors and Lua garbage-collection behavior. Release GPU resources on the owner thread before the window closes.

Proposed world coordinates are right-handed: X right, Y up, and Z toward the viewer in the default camera orientation. Use degrees and document camera direction, field of view, near and far planes, units, and viewport coordinates. Validate these conventions with C and Lua examples before freezing them. Keep historical angles and coordinates in explicit compatibility adapters.

The initial frame contract is: clear; draw the scene in its viewport, including opaque geometry and depth-tested sprites; draw existing 2D process content and persistent text as overlays; apply the existing fade; present. Define clipping and state restoration between passes. Preserve the current order when no scene is active. Start with one active scene and one viewport. Multiple scenes and interleaving with 2D layers need a separate design.

Opaque geometry uses depth testing. For the first sprite milestone, use alpha-tested cutouts with depth writes and document the threshold. Blended transparency needs depth sorting and a defined depth-write policy. Intersecting translucent surfaces cannot always be sorted correctly, so do not promise universal transparency correctness. Test scene occlusion and overlays explicitly.

## Illustrative API shape

These names are examples for discussion, not an approved or implemented API. Use opaque scene and object handles; return checkable C results and convert failures to Lua errors. Do not require Lua for the C path.

```c
/* Illustrative pseudocode: exact types, arguments, and ownership are pending. */
scene = mv_scene_create(context, MV_SCENE_3D);
mv_scene_set_camera(scene, &camera);
sprite = mv_scene_add_sprite(scene, image, &position);
mv_scene_set_sprite_position(scene, sprite, &position);
/* Draw during the scene pass; destroy objects and scene before the window. */
mv_scene_destroy(scene);
```

```lua
-- Illustrative pseudocode: names and ownership are pending.
local scene = mv.scene.create({ mode = "3d" })
scene:set_camera({ position = {0, 5, 10}, target = {0, 0, 0} })
local sprite = scene:add_sprite({ image = image, position = {0, 0, 0} })
sprite:set_position({1, 0, 0})
scene:close()
```

The C and Lua paths must expose equivalent capabilities. Design small mode-specific operations on top of shared camera, resource, and scene concepts. Do not require an `hd2d` preset to create a scene. Decide whether explicit object removal, scene ownership, and automatic process cleanup are separate options before finalizing signatures.

## Milestones

These are planned 2.0 prereleases before beta. Validate the new modes on the platforms claimed after alpha.6 before beta. Each milestone needs C and Lua examples, documentation, cleanup tests, non-visual tests without raylib, and visual smoke tests on verified platforms. Preserve existing 2D tests. Complete each step only with evidence.

1. **2.0.0-alpha.7 — Mode 7 MVP.** Define scene lifecycle, viewport, camera, and ordering. Draw one textured flat floor, a sky color, and camera-facing sprites with distance ordering. Exercise movement, rotation, height, horizon, field of view, and distance. Start with a texture; tilemaps and advanced sky are later work. Compare projection math headlessly and test pixels with a graphics backend.
2. **2.0.0-alpha.8 — Mode 9 MVP.** Add shared 3D camera and transforms, a textured mesh or a minimal model, and an animated billboard with cutout transparency and depth occlusion. Support scene destruction and recreation. Test C and Lua examples and 2D overlay order. Select and document the first model format after testing its loader and dependencies; glTF and OBJ support are not assumed.
3. **2.0.0-alpha.9 — Mode 8 MVP.** Define a backend-independent sector model with validated floors, ceilings, walls, and portals. Render two connected rooms, include sprites and a first-person camera, and add basic wall and floor collision queries. Test invalid topology, portal traversal, heights, and collision headlessly. Reuse the 3D renderer if the prototype meets the tests; do not assume it does.
4. **2.0.0-beta.1 and 2.0.0 — stabilization.** Complete the three MVP contracts, platform matrix, regression tests, examples, and documentation. Freeze only the verified feature set. Resolve failed gates before release.

Advanced lighting, fog, skyboxes, low-resolution rendering, blended sprites, retro effects, HD-2D postprocessing, more model formats, moving sectors, and character controllers are candidates for a later release. Schedule them separately after measurements and user feedback. Collision queries and a reusable controller are distinct features. Define slopes and stairs with their own tests before claiming support.

## Compatibility and tooling

Modern Mode 7 does not certify historical Mode 7 semantics. Record signatures, units, regions, error behavior, and reference traces before adding a compatibility adapter. Mode 8 `.WLD` import and DIV Mode9 compatibility are separate research projects with fixtures and license review. CLI and GUI scene tooling follows the runtime contracts; it is not a prerequisite for the MVPs.

## Risks and decisions still open

- Choose whether scene CPU data lives in the core or in a separate backend-independent library. Prove the C-only build remains independent of Lua and raylib.
- Specify resource sharing, context/window association, teardown order, process death, and stale handles before implementing a public API.
- Test viewport clipping, depth state, transparent geometry, texture filtering, and supported GPU targets. A shader or raylib 3D path is a technical choice, not the public contract.
- Define map bounds, near-horizon behavior, sprite projection and occlusion for Mode 7. A flat perspective floor is not a full terrain renderer.
- Sector topology, triangulation, portals, vertical movement, and collision are separate problems. Decide supported geometry and invalid inputs before authoring tools.
- Model format choice needs tested asset fixtures, material limits, animation scope, and licensing. Do not imply complete glTF support from loading one sample.
- Headless tests validate scene state, math, and collision, not rendered pixels. Run graphics checks on the backend and platform actually claimed.
