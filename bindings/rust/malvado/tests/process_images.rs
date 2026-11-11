// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Engine, Error, Signal,
    graphic::{Geometry, Graphic},
    image::Image,
    window::{Color, Window},
};
use std::rc::Rc;
unsafe extern "C" {
    fn malvado_test_pixel(x: i32, y: i32, r: i32, g: i32, b: i32) -> i32;
}
fn main() {
    for mode in ["clear", "primitive", "kill", "return", "context"] {
        let window = Rc::new(Window::open(64, 64, "Rust process image owner").unwrap());
        let weak = Rc::downgrade(&window);
        let image = Image::from_rgba(window.clone(), 2, 2, &[255, 0, 0, 255].repeat(4)).unwrap();
        let sprite = image.sprite((1, 1));
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
        let alias = actor.clone();
        assert!(actor.sprite_geometry().unwrap().is_none());
        if mode == "clear" {
            let png = Image::from_png(
                window.clone(),
                include_bytes!("../../../../tests/fixtures/red.png"),
            )
            .unwrap();
            actor.set_sprite(&png.sprite((4, 4))).unwrap();
            drop(png);
            assert_eq!(actor.sprite_geometry().unwrap().unwrap().region.width, 8);
            let atlas = malvado::atlas::Atlas::parse(r#"{"schema":1,"kind":"graphics-library","image":"unused.png","sprites":[{"id":7,"name":"tile","x":1,"y":0,"width":1,"height":1,"pivot_x":-1,"pivot_y":2}]}"#,2,2).unwrap();
            let texture = malvado::atlas::TextureAtlas::new(atlas, image.clone()).unwrap();
            actor.set_sprite(&texture.sprite(7).unwrap()).unwrap();
            drop(texture);
            assert_eq!(actor.sprite_geometry().unwrap().unwrap().pivot, (-1, 2));
            let fpg = malvado::fpg::Fpg::parse(include_bytes!(
                "../../../../tests/fixtures/fpg/bennugd-8.fpg"
            ))
            .unwrap();
            let textures = malvado::fpg::Textures::new(window.clone(), fpg, true).unwrap();
            actor.set_sprite(&textures.sprite(7).unwrap()).unwrap();
            drop(textures);
            assert_eq!(actor.sprite_geometry().unwrap().unwrap().pivot, (1, 0));
        }
        actor.set_sprite(&sprite).unwrap();
        actor.set_sprite(&sprite).unwrap(); // Replacing an owner keeps the new one live.
        let geometry = alias.sprite_geometry().unwrap().unwrap();
        assert_eq!(geometry.pivot, (1, 1));
        assert_eq!(geometry.region.width, 2);
        assert!(actor.graphic().unwrap().is_none());
        actor.set_position(20.0, 20.0).unwrap();
        actor.set_scale(4.0).unwrap();
        let target = engine
            .spawn(Default::default(), |p| async move {
                loop {
                    p.frame().await;
                }
            })
            .unwrap();
        target
            .set_graphic(Graphic {
                geometry: Geometry::circle(1.0).unwrap(),
                color: Color {
                    r: 0,
                    g: 0,
                    b: 0,
                    a: 0,
                },
            })
            .unwrap();
        target.set_position(20.0, 20.0).unwrap();
        assert!(actor.collides_with(&target).unwrap());
        actor.set_position(40.0, 40.0).unwrap();
        assert!(!actor.collides_with(&target).unwrap());
        actor.set_position(20.0, 20.0).unwrap();

        engine.signal(&actor, Signal::Freeze).unwrap();
        let frame = window
            .frame(Color {
                r: 0,
                g: 0,
                b: 0,
                a: 255,
            })
            .unwrap();
        frame.process_graphics(&engine).unwrap();
        // SAFETY: live main-thread frame; helper only reads a rendered pixel.
        assert_eq!(unsafe { malvado_test_pixel(20, 20, 255, 0, 0) }, 1);
        drop(frame);
        engine.signal(&actor, Signal::Wake).unwrap();
        drop(sprite);
        drop(image);
        drop(window);
        assert!(weak.upgrade().is_some());
        assert!(matches!(
            Window::open(32, 32, "still owned"),
            Err(Error::Busy)
        ));
        match mode {
            "clear" => {
                alias.clear_graphic().unwrap();
                assert!(actor.sprite_geometry().unwrap().is_none());
            }
            "primitive" => {
                actor
                    .set_graphic(Graphic {
                        geometry: Geometry::circle(2.0).unwrap(),
                        color: Color {
                            r: 0,
                            g: 255,
                            b: 0,
                            a: 255,
                        },
                    })
                    .unwrap();
            }
            "kill" => {
                actor.kill().unwrap();
                engine.tick().unwrap();
                assert_eq!(actor.sprite_geometry(), Err(Error::NotFound));
            }
            "return" => {
                engine.tick().unwrap();
                assert_eq!(actor.sprite_geometry(), Err(Error::NotFound));
            }
            "context" => {
                drop(engine);
                assert_eq!(actor.sprite_geometry(), Err(Error::Closed));
            }
            _ => unreachable!(),
        }
        assert!(weak.upgrade().is_none(), "window retained after {mode}");
    }
    let window = Rc::new(Window::open(32, 32, "Failed assignment").unwrap());
    let weak = Rc::downgrade(&window);
    let sprite = Image::from_rgba(window.clone(), 1, 1, &[255; 4])
        .unwrap()
        .sprite((0, 0));
    let mut engine = Engine::new().unwrap();
    let actor = engine
        .spawn(Default::default(), |_| async { Ok(()) })
        .unwrap();
    engine.tick().unwrap();
    let before = Rc::strong_count(&window);
    assert_eq!(actor.set_sprite(&sprite), Err(Error::NotFound));
    assert_eq!(Rc::strong_count(&window), before);
    drop(engine);
    assert_eq!(actor.set_sprite(&sprite), Err(Error::Closed));
    drop(sprite);
    drop(window);
    assert!(weak.upgrade().is_none());
    let window = Rc::new(Window::open(32, 32, "Shared graph owners").unwrap());
    let weak = Rc::downgrade(&window);
    let sprite = Image::from_rgba(window.clone(), 1, 1, &[255; 4])
        .unwrap()
        .sprite((0, 0));
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
    a.set_sprite(&sprite).unwrap();
    b.set_sprite(&sprite).unwrap();
    drop(sprite);
    drop(window);
    drop(first);
    assert!(weak.upgrade().is_some());
    drop(second);
    assert!(weak.upgrade().is_none());
}
