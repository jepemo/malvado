#!/usr/bin/env python3
"""Verify Rust window services against relocated graphics SDKs (needs a display)."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
sys.dont_write_bytecode = True
from test_rust import ROOT, run

WORK = ROOT / "build-rust-graphics"
MANIFEST = str(ROOT / "bindings/rust/Cargo.toml")


def consumer(prefix, profile):
    game = WORK / "external graphics game"
    (game / "src").mkdir(parents=True, exist_ok=True)
    sdk = prefix / "share/malvado/rust/malvado"
    (game / "Cargo.toml").write_text(
        '[package]\nname="graphics-consumer"\nversion="0.0.0"\nedition="2024"\n'
        '[dependencies]\nmalvado={path=' + json.dumps(os.path.relpath(sdk, game))
        + ',features=["graphics","assets"]}\n')
    for name in ("AnonymousPro-Bold.ttf", "OFL.txt"):
        shutil.copyfile(ROOT / "tests/fixtures/fonts" / name, game / name)
    shutil.copyfile(ROOT / "tests/fixtures/silence.wav", game / "silence.wav")
    shutil.copyfile(ROOT / "tests/fixtures/fpg/bennugd-8.fpg", game / "sprites.fpg")
    shutil.copyfile(ROOT / "tests/fixtures/red.png", game / "red.png")
    (game / "src/main.rs").write_text(
        'use malvado::{Engine, graphic::{Graphic, Geometry}, Error, drawing::Rectangle, image::Image, font::Font, window::{Window, Color}};\n'
        'use std::rc::Rc;\n'
        'fn main() -> Result<(), Error> {\n'
        ' let mut fade = malvado::fade::Fade::new()?;\n'
        ' fade.enqueue(Color {r:0,g:0,b:0,a:255}, 2)?; assert!(fade.step()); assert_eq!(fade.color().a, 127);\n'
        ' let audio = Rc::new(malvado::audio::Audio::open()?);\n'
        ' let sound = malvado::audio::Sound::load(audio.clone(), "silence.wav")?;\n'
        ' let copy = sound.try_clone()?; sound.play()?; copy.play()?; sound.stop()?;\n'
        ' copy.pause()?; audio.update()?; copy.resume()?; copy.stop()?; drop(copy); drop(sound);\n'
        ' let music = malvado::audio::Music::load(audio.clone(), "silence.wav", true)?;\n'
        ' music.play()?; audio.update()?; music.pause()?; music.resume()?; music.stop()?; drop(music); drop(audio);\n'
        ' let window = Rc::new(Window::open(160, 120, "Installed Rust window")?);\n'
        ' assert_eq!(window.size(), (160,120));\n'
        ' window.input_tick();\n'
        ' let _position = window.mouse_position();\n'
        ' assert!(!window.pad_connected(255));\n'
        ' assert!(!window.pad_button_down(255, malvado::window::PadButton::A));\n'
        ' assert_eq!(window.pad_axis(255, malvado::window::PadAxis::LeftX), 0.0);\n'
        ' assert_eq!(window.text_input_next(), None);\n'
        ' assert!(!window.key_pressed(malvado::window::Key::Space));\n'
        ' assert!(!window.key_released(malvado::window::Key::Space));\n'
        ' let decoded = Image::from_png(window.clone(), &std::fs::read("red.png").unwrap())?; assert_eq!(decoded.size(),(8,8)); drop(decoded);\n'
        ' let image = Image::from_rgba(window.clone(), 1, 1, &[255,0,0,255])?;\n'
        ' let atlas = malvado::atlas::Atlas::parse(r#"{"schema":1,"kind":"graphics-library","image":"unused.png","sprites":[{"id":7,"name":"red","x":0,"y":0,"width":1,"height":1}]}"#,1,1)?;\n'
        ' let textured = malvado::atlas::TextureAtlas::new(atlas,image.clone())?; assert_eq!(textured.metadata().len(),1); assert_eq!(textured.image().size(),(1,1)); let sprite = textured.sprite(7).unwrap(); assert!(textured.sprite_named("red")?.is_some()); sprite.shape()?; let _ = (sprite.region(),sprite.pivot()); drop(textured);\n'
        ' let fpg = malvado::fpg::Fpg::parse(&std::fs::read("sprites.fpg").unwrap())?; assert_eq!(fpg.pivot(7),Some((1,0))); std::fs::write("library.json",fpg.json(true)?).unwrap(); for i in 0..fpg.len() {let id=fpg.at(i).unwrap().info.id; std::fs::write(format!("{id}.png"),fpg.png(id,true)?).unwrap();} let converted = malvado::fpg::Textures::load(window.clone(),malvado::asset::Library::open("library.json",None)?)?; let malvado::asset::Metadata::Manifest(m)=converted.metadata() else {unreachable!()}; assert_eq!(m.pivot(7),Some((1,0))); drop(converted); let loaded=malvado::fpg::Textures::load(window.clone(),malvado::asset::Library::open("sprites.fpg",None)?)?; drop(loaded); let textures = malvado::fpg::Textures::new(window.clone(),fpg,true)?; assert_eq!(textures.len(),2); assert!(!textures.is_empty()); let _ = textures.metadata(); let _ = textures.at(1); let fpg_sprite = textures.sprite(7).unwrap(); drop(textures);\n'
        ' let font = Font::load(window.clone(), "AnonymousPro-Bold.ttf", 24)?;\n'
        ' assert!(font.text_width("Café", 24)? > 0);\n'
        ' let mut state = malvado::drawing::DrawState::new()?; state.set_background((10,20,30));\n'
        ' state.set_color(Color {r:255,g:0,b:0,a:255}); let frame = window.frame(state.background())?;\n'
        ' frame.font_text(&font, "Café", (10,70), 24, Color {r:255,g:255,b:255,a:255})?; drop(font);\n'
        ' frame.sprite(&fpg_sprite, (70.0,50.0), 1.0, 0.0)?; drop(fpg_sprite); frame.sprite(&sprite, (60.0,50.0), 1.0, 0.0)?; drop(sprite); frame.image(&image, (50,50))?; drop(image);\n'
        ' frame.with_clip(Rectangle {x:0,y:0,width:80,height:60}, |frame| {\n'
        '     frame.rectangle(Rectangle {x:10,y:10,width:20,height:20}, true, state.color())\n'
        ' })?;\n'
        ' let geometry = malvado::graphic::Geometry::rectangle(10.0,10.0,malvado::graphic::Pivot::TopLeft)?;\n'
        ' let engine = Engine::new()?; let actor = engine.spawn(Default::default(), |p| async move {loop {p.frame().await;}})?; actor.set_graphic(Graphic { geometry: Geometry::circle(3.0)?, color: state.color() })?; frame.process_graphics(&engine)?; actor.clear_graphic()?;\n'
        ' let camera = engine.create_camera(malvado::camera::Camera::new(0.0,0.0,None)?)?; actor.set_camera(malvado::camera::Selection::Explicit(camera.clone()))?; frame.process_graphics(&engine)?; camera.close()?; actor.set_camera(malvado::camera::Selection::Main)?;\n'
        '    let region = malvado::camera::Viewport { x: 1, y: 2, width: 30, height: 40 }; actor.set_region(Some(region))?; assert_eq!(actor.region()?, Some(region)); actor.set_scroll((3.0,4.0))?; assert_eq!(actor.scroll()?, (3.0,4.0)); assert_eq!(actor.view_offset()?, (-3.0,-4.0)); assert_eq!(actor.view_clip(None,(100,80))?, Some(region)); actor.set_region(None)?; actor.set_scroll((0.0,0.0))?;\n'
        ' frame.graphic(geometry, malvado::collision::Transform::default(), Color {r:255,g:0,b:0,a:255})?;\n'
        ' frame.fade(&fade)?; drop(frame);\n'
        ' Ok(())\n}\n')
    run("cargo", "run", "--offline", *(["--release"] if profile == "Release" else []), cwd=game,
        env=dict(os.environ, MALVADO_SDK_PREFIX=str(prefix)), timeout=60)


def capture(command, env=None):
    result = subprocess.run(command, env=env, cwd=ROOT, text=True, capture_output=True, timeout=180)
    print(result.stdout, end="", flush=True)
    print(result.stderr, end="", file=sys.stderr, flush=True)
    result.check_returncode()
    return result.stdout


def parity(output, prefix):
    expected = (ROOT / "bindings/rust/malvado/tests/fixtures/keyboard.txt").read_text().splitlines() * 2
    actual = [line.removeprefix(prefix) for line in output.splitlines() if line.startswith(prefix)]
    if actual != expected:
        raise RuntimeError(f"{prefix} keyboard trace mismatch: {actual!r}")


def rejected(prefix, expected):
    env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix),
               CARGO_TARGET_DIR=str(WORK / "rejections"))
    result = subprocess.run(["cargo", "check", "--manifest-path", MANIFEST,
                             "-p", "malvado", "--features", "graphics"],
                            env=env, text=True, capture_output=True)
    if result.returncode == 0 or expected not in result.stdout + result.stderr:
        raise RuntimeError(f"expected graphics SDK rejection {expected!r}\n{result.stdout}\n{result.stderr}")
    print(f"PASS: {expected}", flush=True)


def main():
    if sys.platform.startswith("linux"):
        os.environ["ALSA_CONFIG_PATH"] = str(ROOT / "tests/alsa-null.conf")
    for profile in ("Debug", "Release"):
        build = WORK / profile / "native"
        staged = WORK / profile / "staged"
        prefix = WORK / profile / "relocated graphics sdk"
        args = ["cmake", "-S", str(ROOT), "-B", str(build),
                f"-DCMAKE_BUILD_TYPE={profile}", "-DMALVADO_BUILD_LUA=ON",
                "-DMALVADO_BUILD_RAYLIB=ON", "-DMALVADO_VISUAL_TESTS=ON",
                "-DCMAKE_INSTALL_LIBDIR=lib"]
        # Reuse verified FetchContent sources when a host suite ran first.
        for name in ("lua", "raylib", "cjson"):
            source = ROOT / "build-visual/_deps" / f"{name}-src"
            if source.is_dir():
                args.append(f"-DFETCHCONTENT_SOURCE_DIR_{name.upper()}={source}")
        run(*args)
        run("cmake", "--build", str(build), "--parallel")
        run("cmake", "--install", str(build), "--prefix", str(staged))
        if prefix.exists():
            shutil.rmtree(prefix)
        staged.rename(prefix)
        env = dict(os.environ, MALVADO_SDK_PREFIX=str(prefix),
                   CARGO_TARGET_DIR=str(WORK / "target"))
        cargo = ["cargo", "test", "--manifest-path", MANIFEST, "--workspace", "--all-features"]
        if profile == "Release":
            cargo.append("--release")
        output = capture(cargo, env=env)
        parity(output, "rust-key: ")
        output = capture([str(build / "key-edges-visual-test"), str(ROOT / "tests/key_edges_visual.lua")])
        parity(output, "native-key: ")
        parity(output, "lua-key: ")
        if sys.platform.startswith("linux"):
            if shutil.which("xdotool") is None:
                raise RuntimeError("Linux keyboard injection requires xdotool")
            for target, scripts in [
                ("keyboard", [("keys_visual.sh", "Malvado Rust keys", "--physical")]),
                ("input", [("mouse_visual.sh", "Malvado Rust mouse", "--mouse"),
                           ("text_input_visual.sh", "Malvado Rust text input", "--text")]),
            ]:
                command = ["cargo", "test", "--manifest-path", MANIFEST, "-p", "malvado",
                           "--all-features", "--test", target, "--no-run", "--message-format=json"]
                if profile == "Release": command.append("--release")
                artifacts = capture(command, env=env)
                executable = next(json.loads(line)["executable"] for line in artifacts.splitlines()
                                  if line.startswith("{") and json.loads(line).get("executable"))
                for script, title, mode in scripts:
                    run("sh", str(ROOT / "tests" / script), title, executable, mode, env=env, timeout=30)
        run("cargo", "clippy", "--manifest-path", MANIFEST, "--workspace",
            "--all-targets", "--all-features", "--", "-D", "warnings", env=env)
        consumer(prefix, profile)
    # Verify feature discovery even when this task runs before the core suite.
    core = WORK / "core-only"
    run("cmake", "-S", str(ROOT), "-B", str(core), "-DMALVADO_BUILD_LUA=OFF",
        "-DMALVADO_BUILD_RAYLIB=OFF", "-DMALVADO_BUILD_ASSETS=OFF",
        "-DCMAKE_INSTALL_LIBDIR=lib", "-DFETCHCONTENT_FULLY_DISCONNECTED=ON")
    run("cmake", "--build", str(core), "--target", "malvado_core", "--parallel")
    run("cmake", "--install", str(core), "--prefix", str(core / "sdk"))
    rejected(core / "sdk", "Malvado_FOUND to FALSE")
    drift = WORK / "drift sdk"
    if drift.exists(): shutil.rmtree(drift)
    shutil.copytree(prefix, drift)
    header = drift / "include/malvado/graphic.h"
    header.write_text(header.read_text().replace("unsigned char r, g, b, a;", "unsigned int r, g, b, a;"))
    rejected(drift, "native ABI layout/value mismatch")
    run("cargo", "fmt", "--manifest-path", MANIFEST, "--all", "--", "--check")


if __name__ == "__main__":
    main()
