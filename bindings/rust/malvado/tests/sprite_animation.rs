// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Engine, Error, Signal, SpawnOptions,
    atlas::SpriteFrame,
    graphic::{Geometry, Graphic},
    image::Image,
    window::{Color, Window},
};
use std::{cell::Cell, rc::Rc};
unsafe extern "C" {
    fn malvado_test_pixel(x: i32, y: i32, r: i32, g: i32, b: i32) -> i32;
}
fn frames(window: Rc<Window>) -> Vec<SpriteFrame> {
    let red = Image::from_rgba(window.clone(), 2, 2, &[255, 0, 0, 255].repeat(4)).unwrap();
    let green = Image::from_rgba(window, 4, 2, &[0, 255, 0, 255].repeat(8)).unwrap();
    vec![
        SpriteFrame {
            sprite: red.sprite((0.0, 0.0)).unwrap(),
            duration_us: 50000,
        },
        SpriteFrame {
            sprite: green.sprite((0.0, 0.0)).unwrap(),
            duration_us: 50000,
        },
    ]
}
fn main() {
    let window = Rc::new(Window::open(64, 64, "Rust native sprite animation").unwrap());
    let weak = Rc::downgrade(&window);
    let mut engine = Engine::new().unwrap();
    let polls = Rc::new(Cell::new(0));
    let count = polls.clone();
    let actor = engine
        .spawn(
            SpawnOptions {
                fps: 15,
                ..Default::default()
            },
            move |p| async move {
                loop {
                    count.set(count.get() + 1);
                    p.frame().await;
                }
            },
        )
        .unwrap();
    let target = engine
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    target
        .set_graphic(Graphic {
            geometry: Geometry::circle(0.25).unwrap(),
            color: Color {
                r: 0,
                g: 0,
                b: 0,
                a: 0,
            },
        })
        .unwrap();
    actor.set_position(20.0, 20.0).unwrap();
    target.set_position(23.0, 21.0).unwrap();
    {
        let root = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../../tests/fixtures");
        let atlas = malvado::atlas::TextureAtlas::load(
            window.clone(),
            malvado::asset::Library::open(root.join("atlas.json"), None).unwrap(),
        )
        .unwrap();
        let imported = vec![
            SpriteFrame {
                sprite: atlas.sprite(7).unwrap(),
                duration_us: 50000,
            },
            SpriteFrame {
                sprite: atlas.sprite(42).unwrap(),
                duration_us: 50000,
            },
        ];
        actor.set_sprite_animation(&imported, true).unwrap();
        assert_eq!(
            actor.sprite_geometry().unwrap().unwrap().region,
            atlas.sprite(7).unwrap().region()
        );
        drop(imported);
        drop(atlas);
        let fpg = malvado::fpg::Textures::load(
            window.clone(),
            malvado::asset::Library::open(root.join("fpg/bennugd-8.fpg"), None).unwrap(),
        )
        .unwrap();
        let imported = vec![
            SpriteFrame {
                sprite: fpg.sprite(7).unwrap(),
                duration_us: 50000,
            },
            SpriteFrame {
                sprite: fpg.sprite(42).unwrap(),
                duration_us: 50000,
            },
        ];
        actor.set_sprite_animation(&imported, true).unwrap();
        assert_eq!(actor.sprite_geometry().unwrap().unwrap().pivot, (1.0, 0.0));
    }
    let mut sequence = frames(window.clone());
    actor.set_sprite_animation(&sequence, true).unwrap();
    let alias = actor.clone();
    sequence[1].duration_us = 0;
    assert_eq!(
        actor.set_sprite_animation(&sequence, true),
        Err(Error::InvalidArgument)
    );
    assert_eq!(
        actor.set_sprite_animation(&[], true),
        Err(Error::InvalidArgument)
    );
    sequence[1].duration_us = 50000;
    drop(sequence);
    assert_eq!(actor.clear_animation_clock(), Err(Error::Busy));
    let metadata = malvado::animation::Animation::new(
        &[malvado::animation::Frame {
            graph_id: 99,
            duration_us: 50000,
        }],
        true,
    )
    .unwrap();
    assert_eq!(actor.set_animation_clock(&metadata), Err(Error::Busy));
    drop(metadata);

    let trace: Vec<_> = include_str!("../../../../tests/fixtures/process-animation.txt")
        .lines()
        .collect();
    for (i, line) in trace.iter().enumerate() {
        let row: Vec<u64> = line
            .split_whitespace()
            .map(|x| x.parse().unwrap())
            .collect();
        let clock = alias.animation_clock().unwrap().unwrap();
        assert_eq!(clock.elapsed_us, row[0]);
        let green = row[1] == 42;
        assert_eq!(clock.sample.index, usize::from(green));
        assert_eq!(
            actor.sprite_geometry().unwrap().unwrap().region.width,
            if green { 4 } else { 2 }
        );
        assert!(actor.collides_with(&target).unwrap());
        let frame = window
            .frame(Color {
                r: 0,
                g: 0,
                b: 0,
                a: 255,
            })
            .unwrap();
        frame.process_graphics(&engine).unwrap();
        // SAFETY: live main-thread frame; helper only reads rendered pixels.
        assert_eq!(
            unsafe {
                malvado_test_pixel(
                    20,
                    20,
                    if green { 0 } else { 255 },
                    if green { 255 } else { 0 },
                    0,
                )
            },
            1
        );
        drop(frame);
        if i == 4 {
            let invalid = SpriteFrame {
                sprite: Image::from_rgba(window.clone(), 1, 1, &[255; 4])
                    .unwrap()
                    .sprite((1e30, 1e30))
                    .unwrap(),
                duration_us: 50000,
            };
            assert_eq!(
                actor.set_sprite_animation(&[invalid], true),
                Err(Error::InvalidArgument)
            );
        }
        if i + 1 < trace.len() {
            engine.tick().unwrap();
        }
    }
    assert_eq!(polls.get(), 2);
    for signal in [Signal::Freeze, Signal::Sleep] {
        engine.signal(&actor, signal).unwrap();
        engine.tick().unwrap();
        assert_eq!(actor.animation_clock().unwrap().unwrap().elapsed_us, 0);
    }
    engine.signal(&actor, Signal::Wake).unwrap();
    let sequence = frames(window.clone());
    actor.set_sprite_animation(&sequence, false).unwrap();
    drop(sequence);
    for _ in 0..10 {
        engine.tick().unwrap();
    }
    assert_eq!(actor.animation_clock().unwrap().unwrap().elapsed_us, 100000);
    assert_eq!(actor.sprite_geometry().unwrap().unwrap().region.width, 4);
    drop(window);
    assert!(weak.upgrade().is_some());
    alias.clear_graphic().unwrap();
    assert!(weak.upgrade().is_none());
    assert!(actor.animation_clock().unwrap().is_none());
    for mode in ["kill", "return", "context", "static"] {
        let window = Rc::new(Window::open(32, 32, "Animated graph cleanup").unwrap());
        let weak = Rc::downgrade(&window);
        let sequence = frames(window.clone());
        let mut engine = Engine::new().unwrap();
        let natural = mode == "return";
        let actor = engine
            .spawn(Default::default(), move |p| async move {
                if !natural {
                    loop {
                        p.frame().await;
                    }
                }
                Ok(())
            })
            .unwrap();
        actor.set_sprite_animation(&sequence, true).unwrap();
        let static_sprite = sequence[0].sprite.clone();
        drop(sequence);
        if mode == "static" {
            actor.set_sprite(&static_sprite).unwrap();
            assert!(actor.animation_clock().unwrap().is_none());
        }
        drop(static_sprite);
        drop(window);
        assert!(weak.upgrade().is_some());
        match mode {
            "kill" => {
                actor.kill().unwrap();
                engine.tick().unwrap();
            }
            "return" => {
                engine.tick().unwrap();
            }
            "static" => {
                actor.clear_graphic().unwrap();
            }
            "context" => {
                drop(engine);
            }
            _ => unreachable!(),
        }
        assert!(weak.upgrade().is_none(), "window retained after {mode}");
    }
    let window = Rc::new(Window::open(32, 32, "Animation failure cleanup").unwrap());
    let weak = Rc::downgrade(&window);
    let sequence = frames(window.clone());
    let mut engine = Engine::new().unwrap();
    let ended = engine
        .spawn(Default::default(), |_| async { Ok(()) })
        .unwrap();
    engine.tick().unwrap();
    assert_eq!(
        ended.set_sprite_animation(&sequence, true),
        Err(Error::NotFound)
    );
    drop(engine);
    assert_eq!(
        ended.set_sprite_animation(&sequence, true),
        Err(Error::Closed)
    );
    drop(sequence);
    drop(window);
    assert!(weak.upgrade().is_none());
    let window = Rc::new(Window::open(32, 32, "Shared animation owners").unwrap());
    let weak = Rc::downgrade(&window);
    let sequence = frames(window.clone());
    let first = Engine::new().unwrap();
    let second = Engine::new().unwrap();
    let a = first
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    let b = second
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    a.set_sprite_animation(&sequence, true).unwrap();
    b.set_sprite_animation(&sequence, true).unwrap();
    drop(sequence);
    drop(window);
    drop(first);
    assert!(weak.upgrade().is_some());
    drop(second);
    assert!(weak.upgrade().is_none());
}
