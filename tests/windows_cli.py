#!/usr/bin/env python3
"""Verify Windows installed CLI authoring and fresh-runner exported players."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import traceback

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from windows_graphics import deploy, prepare

VALUE = b"asset\nvalue\r\n"
C_GAME = r'''
#include <stdio.h>
#include <string.h>
#include <malvado/malvado.h>
#include <malvado/platform.h>
static MvStepResult step(MvContext *ctx, MvProcessId id, void *user) {
    (void)ctx; (void)id;
    return ++*(unsigned *)user == 3 ? MV_STEP_DONE : MV_STEP_YIELD;
}
int main(void) {
    char bytes[32];
    FILE *file = fopen("assets/value.txt", "rb");
    if (!file) return 1;
    size_t size = fread(bytes, 1, sizeof(bytes), file);
    fclose(file);
    if (size != 13 || memcmp(bytes, "asset\nvalue\r\n", 13)) return 1;
    MvContext *ctx = mv_context_create(60, 5);
    if (!ctx || !mv_window_open(160, 120, "Windows C export")) return 1;
    unsigned count = 0;
    MvProcessId id;
    if (mv_process_spawn(ctx, 0, 0, step, NULL, &count, &id) != MV_OK) return 1;
    while (mv_process_count(ctx)) {
        if (mv_process_tick(ctx) != MV_OK) return 1;
        mv_window_begin((MvColor){20, 30, 40, 255});
        mv_window_end();
    }
    mv_context_destroy(ctx);
    mv_window_close();
    printf("C export: %u\n", count);
    return count == 3 ? 0 : 1;
}
'''
LUA_GAME = r'''
local file = assert(io.open('assets/value.txt', 'rb'))
assert(file:read('a') == 'asset\nvalue\r\n')
file:close()
local mv = require('malvado')
local count = 0
local task = mv.process(function()
    for _ = 1, 3 do
        count = count + 1
        mv.draw_text('Windows Lua export', 1, 1, 10)
        mv.frame()
    end
end)
mv.start(function() task() end)
assert(count == 3)
print('Lua export: 3')
'''
RUST_GAME = r'''
use malvado::{Error, Game, SpawnOptions, window::{Window, Color}};
fn main() -> Result<(), Error> {
    assert_eq!(std::fs::read("assets/value.txt").unwrap(), b"asset\nvalue\r\n");
    let window = Window::open(160,120,"Windows Rust export")?;
    let mut game = Game::new()?;
    game.spawn(SpawnOptions::default(), |p| async move {
        for _ in 0..3 { p.frame().await; }
        println!("Rust export: 3");
        Ok(())
    })?;
    while game.process_count() != 0 {
        window.input_tick(); game.tick()?;
        let frame = window.frame(Color::default())?;
        frame.process_graphics(&game)?;
    }
    Ok(())
}
'''


def run(*args, cwd, env=None, success=True, timeout=180):
    print(f">>> {args!r}", flush=True)
    result = subprocess.run([str(arg) for arg in args], cwd=cwd, env=env,
                            text=True, capture_output=True, timeout=timeout)
    print(result.stdout + result.stderr, flush=True)
    if (result.returncode == 0) != success:
        raise RuntimeError(f"Unexpected exit {result.returncode}: {args!r}")
    return result.stdout + result.stderr


def configure_driver(work):
    os.environ["MALVADO_MESA_DIR"] = str(prepare(work / "mesa"))
    os.environ["GALLIUM_DRIVER"] = "llvmpipe"


def author_game(cli, sdk, work, players, host, language, source, filename, label):
    project = work / f"{label} game with spaces"
    run(cli, "new", "--lang", language, project, cwd=work)
    (project / filename).write_text(source, encoding="utf-8")
    (project / "assets").mkdir(exist_ok=True)
    (project / "assets/value.txt").write_bytes(VALUE)
    (project / "assets/pixels.bin").write_bytes(bytes(range(256)))
    if language != "lua":
        run(cli, "build", project, cwd=work)
        binary = project / (f".malvado/rust/{host}/debug/game.exe" if language == "rust"
                            else ".malvado/build/game.exe")
        assert binary.is_file()
        deploy(binary.parent)
    if language != "c":
        run(cli, "check", project, cwd=work)
    assert f"{label} export: 3" in run(cli, "run", project, cwd=work)
    output = players / f"{label} exported game"
    run(cli, "export", output, cwd=project)
    assert output.is_dir()
    for name in ("LICENSE", "Lua-readme.html", "raylib-LICENSE", "cJSON-LICENSE"):
        assert (output / "licenses" / name).is_file(), name
    for asset in ("value.txt", "pixels.bin"):
        assert (output / "assets" / asset).read_bytes() == (project / "assets" / asset).read_bytes()
    assert not (output / "opengl32.dll").exists(), "test driver must not enter exported packages"
    if language != "lua":
        assert (output / "game-bin.exe").is_file()
        assert not any(path.is_file() and (path.suffix in (".c", ".h", ".rs") or
                       path.name in ("Cargo.toml", "Cargo.lock", "CMakeLists.txt"))
                       for path in output.rglob("*"))
    before = (output / "game.exe").read_bytes()
    run(cli, "export", output, cwd=project, success=False)
    assert (output / "game.exe").read_bytes() == before
    # Windows case folding must not allow a reserved executable to replace the player.
    (project / "GAME.EXE").write_bytes(b"reserved")
    rejected = players / f"{label} rejected"
    run(cli, "export", rejected, cwd=project, success=False)
    assert not rejected.exists()
    (project / "GAME.EXE").unlink()
    assert not list(players.glob(".malvado-export-*"))
    # Hide authoring tools without changing the host or permitting Rustup downloads.
    env = dict(os.environ, PATH=str(Path(os.environ["SystemRoot"]) / "System32"))
    if language != "lua":
        run(cli, "build", project, cwd=work, env=env, success=False)
    if language == "rust":
        launcher = sdk / "libexec/malvado/malvado-native-game.exe"
        hidden = launcher.with_suffix(".saved")
        launcher.rename(hidden)
        try:
            assert "required licenses missing" in run(cli, "doctor", cwd=work)
        finally:
            hidden.rename(launcher)
    return {"folder": output.name, "trace": f"{label} export: 3"}


def author(sdk, work, players):
    cli = sdk / "bin/malvado.exe"
    deploy(cli.parent)
    assert "Export: launchers and required licenses found" in run(cli, "doctor", cwd=work)
    host = subprocess.check_output(["rustc", "-vV"], text=True).split("host: ")[1].splitlines()[0]
    manifest = []
    failures = []
    for language, source, filename, label in (
            ("lua", LUA_GAME, "main.lua", "Lua"),
            ("c", C_GAME, "main.c", "C"),
            ("rust", RUST_GAME, "main.rs", "Rust")):
        try:
            manifest.append(author_game(cli, sdk, work, players, host, language, source, filename, label))
        except Exception as error:
            failures.append(f"{label}: {error}")
            traceback.print_exc()
    if failures:
        raise RuntimeError("Windows authoring failures: " + "; ".join(failures))
    (players / "manifest.json").write_text(json.dumps({
        "revision": os.environ.get("GITHUB_SHA", "local"), "games": manifest,
        "assets": {"value.txt": hashlib.sha256(VALUE).hexdigest(),
                   "pixels.bin": hashlib.sha256(bytes(range(256))).hexdigest()},
    }, indent=2), encoding="utf-8")
    print("PASS: Windows C/Lua/Rust create/check/build/run/export, assets, notices and failure paths", flush=True)


def play(players, work):
    manifest = json.loads((players / "manifest.json").read_text(encoding="utf-8"))
    revision = os.environ.get("GITHUB_SHA")
    if revision:
        assert manifest["revision"] == revision
    env = dict(os.environ, PATH=str(Path(os.environ["SystemRoot"]) / "System32"))
    for name in ("MALVADO_SDK_PREFIX", "CARGO_HOME", "RUSTUP_HOME"):
        env.pop(name, None)
    for tool in ("cargo", "rustc", "cl", "cmake", "gcc"):
        assert shutil.which(tool, path=env["PATH"]) is None, tool
    for game in manifest["games"]:
        directory = players / game["folder"]
        for name, expected in manifest["assets"].items():
            assert hashlib.sha256((directory / "assets" / name).read_bytes()).hexdigest() == expected
        deploy(directory)
        assert game["trace"] in run(directory / "game.exe", cwd=work, env=env, timeout=30)
        asset = directory / "assets/value.txt"
        hidden = asset.with_suffix(".saved")
        asset.rename(hidden)
        try:
            run(directory / "game.exe", cwd=work, env=env, success=False, timeout=30)
        finally:
            hidden.rename(asset)
    print("PASS: fresh Windows players run outside their directories without SDK, Cargo, rustc or compilers on PATH", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", type=Path)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--players", type=Path, required=True)
    parser.add_argument("--fresh", action="store_true")
    args = parser.parse_args()
    if os.name != "nt":
        parser.error("This verifier requires native Windows")
    work, players = args.work.resolve(), args.players.resolve()
    work.mkdir(parents=True, exist_ok=True)
    players.mkdir(parents=True, exist_ok=True)
    configure_driver(work)
    if args.fresh:
        play(players, work)
    elif args.sdk:
        author(args.sdk.resolve(), work, players)
    else:
        parser.error("--sdk is required for authoring verification")


if __name__ == "__main__":
    main()
