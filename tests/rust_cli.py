#!/usr/bin/env python3
"""Exercise Rust projects through an installed CLI, outside the source tree."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import os
from pathlib import Path
import subprocess
import shutil
import tomllib


def run(cli, *args, cwd, env=None, code=0):
    result = subprocess.run([str(cli), *map(str, args)], cwd=cwd, env=env,
                            text=True, capture_output=True, timeout=120)
    if result.returncode != code:
        raise AssertionError(f"{args}: expected {code}, got {result.returncode}\n"
                             f"{result.stdout}\n{result.stderr}")
    return result.stdout + result.stderr


def creation(cli, work, visual):
    game = work / "Rust game"
    run(cli, "new", "--lang", "rust", game, cwd=work)
    manifest = tomllib.loads((game / "Cargo.toml").read_text())
    assert manifest["package"]["name"] == "game"
    assert manifest["bin"] == [{"name": "game", "path": "main.rs"}]
    dependency = manifest["dependencies"]["malvado"]
    assert Path(dependency["path"]).is_dir()
    assert dependency["features"] == ["graphics"]
    assert not (game / "malvado.toml").exists()
    before = (game / "main.rs").read_bytes()
    run(cli, "new", "--lang", "rust", game, cwd=work, code=1)
    assert (game / "main.rs").read_bytes() == before
    run(cli, "check", game, cwd=work)
    if visual:
        # Bound the generated host loop without changing the delivered template.
        bounded = before.decode().replace("    while !window.should_close()",
            "    let mut frames = 0;\n    while { frames += 1; frames <= 3 } && !window.should_close()")
        (game / "main.rs").write_text(bounded)
        run(cli, "run", game, cwd=work)
        run(cli, "export", work / "starter player", cwd=game)
        run(work / "starter player/game", cwd=work, env=dict(os.environ, PATH=""))
        (game / "main.rs").write_bytes(before)
    print("PASS: Rust template compiles against installed SDK; existing project preserved", flush=True)
    return game



def commands(cli, work, game):
    doctor = run(cli, "doctor", cwd=work)
    assert "Cargo: found" in doctor and "Rust SDK: " in doctor
    (game / "assets").mkdir()
    (game / "assets/value.txt").write_text("asset")
    (game / "main.rs").write_text('''use malvado::{Engine, Error, SpawnOptions};
fn main() -> Result<(), Error> {
    assert_eq!(std::fs::read_to_string("assets/value.txt").unwrap(), "asset");
    std::fs::write("executed", "yes").unwrap();
    let mut engine = Engine::new()?;
    engine.spawn(SpawnOptions::default(), |p| async move {
        println!("rust: first");
        p.frame().await;
        println!("rust: second");
        Ok(())
    })?;
    engine.tick()?;
    engine.tick()?;
    assert_eq!(engine.process_count(), 0);
    if std::env::var_os("MALVADO_TEST_EXIT").is_some() { std::process::exit(7); }
    Ok(())
}
''')
    (game / ".cargo").mkdir()
    (game / ".cargo/config.toml").write_text('[build]\ntarget = "unsupported-target"\n')
    # The native host target overrides ambient cross-target configuration.
    run(cli, "check", game, cwd=work)
    assert not (game / "executed").exists()
    run(cli, "build", game, cwd=work)
    host = subprocess.check_output(["rustc", "-vV"], text=True).split("host: ")[1].splitlines()[0]
    binary = game / ".malvado/rust" / host / "debug/game"
    assert binary.is_file() and not (game / "executed").exists()
    timestamp = binary.stat().st_mtime_ns
    run(cli, "build", game, cwd=work)
    assert binary.stat().st_mtime_ns == timestamp
    output = run(cli, "run", game, cwd=work)
    assert "rust: first\nrust: second" in output
    assert (game / "executed").read_text() == "yes"
    run(cli, "run", game, cwd=work, env=dict(os.environ, MALVADO_TEST_EXIT="yes"), code=7)
    (game / "executed").unlink()
    run(cli, "run", "--headless", game, cwd=work, code=2)
    for entry in ("main.lua", "main.c"):
        (game / entry).write_text("invalid mixed project")
        if entry == "main.c":
            (game / "CMakeLists.txt").write_text("project(other C)")
        for command in ("run", "build", "check"):
            output = run(cli, command, game, cwd=work, code=2)
            assert "Ambiguous project" in output
        if entry == "main.lua":
            (game / entry).write_text('print("explicit Lua script")')
            assert "explicit Lua script" in run(cli, "run", "--headless", "main.lua", cwd=game)
            run(cli, "check", "main.lua", cwd=game)
        (game / entry).unlink()
        if entry == "main.c":
            (game / "CMakeLists.txt").unlink()
    empty_path = dict(os.environ, PATH="")
    output = run(cli, "doctor", cwd=work, env=empty_path)
    assert "Cargo: not found" in output and "Rust compiler: not found" in output
    run(cli, "build", game, cwd=work, env=empty_path, code=1)
    tools = work / "rustc only"
    tools.mkdir()
    (tools / "rustc").symlink_to(shutil.which("rustc"))
    output = run(cli, "build", game, cwd=work, env=dict(os.environ, PATH=str(tools)), code=1)
    assert "cargo" in output
    manifest = (game / "Cargo.toml").read_text()
    (game / "Cargo.toml").write_text("invalid manifest [")
    run(cli, "check", game, cwd=work, code=1)
    (game / "Cargo.toml").write_text(manifest.replace('name = "game"', 'name = "other"'))
    run(cli, "build", game, cwd=work, code=1)
    (game / "Cargo.toml").write_text(manifest)
    source = (game / "main.rs").read_text()
    (game / "main.rs").write_text("compile_error!(\"invalid source\");")
    run(cli, "check", game, cwd=work, code=1)
    (game / "main.rs").write_text(source)
    output = run(cli, "check", game, cwd=work,
                 env=dict(os.environ, CC="malvado-no-such-compiler"), code=1)
    assert "malvado-no-such-compiler" in output
    # A corrected environment must recover without deleting Cargo outputs.
    run(cli, "check", game, cwd=work)
    protected = work / "unmanaged"
    protected.mkdir()
    shutil.copy(game / "Cargo.toml", protected)
    (protected / ".malvado/rust").mkdir(parents=True)
    (protected / ".malvado/rust/user-file").write_text("keep")
    run(cli, "build", protected, cwd=work, code=1)
    assert (protected / ".malvado/rust/user-file").read_text() == "keep"
    dependency = Path(tomllib.loads(manifest)["dependencies"]["malvado"]["path"])
    sdk_manifest = dependency / "Cargo.toml"
    saved = sdk_manifest.with_suffix(".saved")
    sdk_manifest.rename(saved)
    try:
        run(cli, "new", "--lang", "rust", work / "missing SDK", cwd=work, code=1)
        assert not (work / "missing SDK").exists()
        run(cli, "build", game, cwd=work, code=1)
        assert "Rust SDK: not found" in run(cli, "doctor", cwd=work)
    finally:
        saved.rename(sdk_manifest)
    print("PASS: Rust CLI check/build/run, scheduler trace, incremental build and diagnostics", flush=True)



def exports(cli, work, game):
    # Cargo owns source layout. Export must not assume a root main.rs.
    (game / "src").mkdir()
    (game / "main.rs").rename(game / "src/custom.rs")
    manifest = game / "Cargo.toml"
    manifest.write_text(manifest.read_text().replace('path = "main.rs"', 'path = "src/custom.rs"'))
    (game / "target").mkdir()
    (game / "target/not-an-asset").write_text("skip")
    (game / "assets/ignored.rs").write_text("source")
    (game / "assets/directory.rs").mkdir()
    (game / "assets/directory.rs/pixel.dat").write_text("asset")
    run(cli, "export", cwd=game)
    output = game / "dist/game"
    for name in ("game", "game-bin", "assets/value.txt", "assets/directory.rs/pixel.dat"):
        assert (output / name).is_file(), name
    for name in ("Cargo.toml", "Cargo.lock", "target", ".cargo", ".malvado", "assets/ignored.rs"):
        assert not (output / name).exists(), name
    assert not list(output.rglob("*.rs")) or all(p.is_dir() for p in output.rglob("*.rs"))
    assert (output / "src").is_dir() and not list((output / "src").iterdir())
    for notice in ("LICENSE", "Lua-readme.html", "raylib-LICENSE"):
        assert (output / "licenses" / notice).is_file(), notice
    inspect = ["otool", "-L"] if os.uname().sysname == "Darwin" else ["ldd"]
    dependencies = subprocess.check_output([*inspect, str(output / "game-bin")], text=True)
    # otool includes the inspected filename on its first line.
    if os.uname().sysname == "Darwin":
        dependencies = "\n".join(dependencies.splitlines()[1:])
    assert str(cli.parent.parent) not in dependencies and "not found" not in dependencies
    original = (output / "game-bin").read_bytes()
    run(cli, "export", cwd=game, code=1)
    assert (output / "game-bin").read_bytes() == original
    moved = work / "moved player"
    shutil.copytree(output, moved)
    # The player needs neither the SDK nor authoring tools on PATH.
    prefix = cli.parent.parent
    hidden = prefix.with_name(prefix.name + " hidden")
    prefix.rename(hidden)
    try:
        player = run(moved / "game", cwd=work, env=dict(os.environ, PATH=""))
        assert "rust: first\nrust: second" in player
        assert (moved / "executed").read_text() == "yes"
    finally:
        hidden.rename(prefix)
    if os.uname().sysname == "Darwin":
        run(cli, "export", "--app", cwd=game)
        app = game / "dist/game.app/Contents/MacOS"
        assert "rust: first\nrust: second" in run(app / "game", cwd=work, env=dict(os.environ, PATH=""))
    else:
        run(cli, "export", "--app", cwd=game, code=1)
    symlink = game / "assets/link.rs"
    symlink.symlink_to(game / "assets/value.txt")
    try:
        run(cli, "export", work / "rejected link", cwd=game, code=1)
        assert not (work / "rejected link").exists()
    finally:
        symlink.unlink()
    (game / "game-bin").write_text("keep")
    run(cli, "export", work / "reserved", cwd=game, code=1)
    assert (game / "game-bin").read_text() == "keep" and not (work / "reserved").exists()
    (game / "game-bin").unlink()
    source = game / "src/custom.rs"
    original_source = source.read_text()
    source.write_text('compile_error!("no stale exports");')
    run(cli, "export", work / "failed build", cwd=game, code=1)
    assert not (work / "failed build").exists()
    source.write_text(original_source)
    assert not list(work.glob(".malvado-export-*"))
    print("PASS: Rust folder/app export, custom Cargo entry, assets, source exclusion and player without SDK/tools", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cli", type=Path, required=True)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--visual", action="store_true")
    args = parser.parse_args()
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    cli = args.cli.resolve()
    game = creation(cli, work, args.visual)
    commands(cli, work, game)
    exports(cli, work, game)


if __name__ == "__main__":
    main()
