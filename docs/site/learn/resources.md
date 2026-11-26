# Resources and assets

Load one atlas for two actors. Then launch the same program from a
different working directory and watch it find the same two asset files.

## Run the example

Install the [desktop SDK](../../start/installation/) and extract the ZIP
into an empty directory. It includes `sprites.lua`, the local
`assets/sheet.json` and `assets/sheet.png`, the Apache 2.0 license and
credits. Open a terminal in the extracted directory.

```sh
malvado run sprites.lua
```

Two sprites from the same atlas appear. Press Escape or close the window
to exit. Then change to the parent directory and rerun using the
entry path. For example, if the extracted directory is named `lesson`,
run `malvado run lesson/sprites.lua`. Both launches show the same sprites.

<!-- example -->

## Find the file that will be opened

By default, the Lua runner resolves asset paths from the game directory
it selects for the entry point. In this example the game table sets
`assets_dir = "assets"`, so `load_asset("sheet.json")` opens
`assets/sheet.json` beside `sprites.lua`. If you write
`load_asset("assets/sheet.json")` with that setting, the path includes
`assets` twice.

Use `get_assets_dir()` to see the effective absolute path.
`set_assets_dir("assets")` changes the base for later loads. A relative
override starts at the game directory. An absolute filename uses its
own path. An atlas resolves its image reference from the directory that
contains the metadata file. Move its JSON and PNG together.

Try changing `assets_dir` to `"missing"` and run again. The load fails.
Restore the setting and check that the example runs. Changing the asset
directory leaves the working directory, Lua module search and existing
resources alone. Include the required files when exporting a game.

## Keep resources alive

Load the atlas once during initialization. Each process holds the shared
sprite descriptor in its graph, and the descriptor keeps its library and
image alive. Clearing an actor's graph removes that actor's reference.
Lua can collect the resource after the last reference goes away. A queued
draw command, retained process handle, or active playback can also keep a
resource alive. Delete persistent text or stop playback when it should
end before ordinary cleanup.

In C, release directly owned images, fonts and audio objects before closing
their platform service. Pair each managed retain with a release. In
experimental Rust, safe wrappers release their references when dropped.

See [game asset paths for Lua, C and Rust](https://github.com/jepemo/malvado/blob/main/docs/api/asset-sources.md#game-asset-paths),
the [Lua image and asset reference](../../reference/lua/images/) and the
[resource lifetime guide](../../concepts/resource-lifetime/).
