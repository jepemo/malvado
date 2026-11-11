// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Engine, Error,
    drawing::Rectangle,
    image::{Image, Transform},
    window::{Color, Window},
};
use std::{cell::RefCell, path::Path, rc::Rc};
unsafe extern "C" {
    fn malvado_test_pixel(x: i32, y: i32, r: i32, g: i32, b: i32) -> i32;
}
fn pixel(x: i32, y: i32, color: Color) {
    // SAFETY: active main-thread frame; the test helper copies a pixel only.
    assert_eq!(
        unsafe { malvado_test_pixel(x, y, color.r.into(), color.g.into(), color.b.into()) },
        1,
        "pixel at {x},{y}"
    );
}
fn main() {
    let window = Rc::new(Window::open(160, 120, "Rust image ownership").unwrap());
    let weak = Rc::downgrade(&window);
    let fixture = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../../tests/fixtures/red.png");
    assert!(Image::load(window.clone(), fixture.with_file_name("missing.png")).is_err());
    assert!(matches!(
        Image::load(window.clone(), "bad\0path"),
        Err(Error::InvalidArgument)
    ));
    for (width, height, bytes) in [(0, 2, 16), (2, -1, 16), (8193, 1, 16), (2, 2, 15)] {
        assert!(matches!(
            Image::from_rgba(window.clone(), width, height, &vec![0; bytes]),
            Err(Error::InvalidArgument)
        ));
    }
    let mut png = std::fs::read(&fixture).unwrap();
    for length in 0..24 {
        assert!(Image::from_png(window.clone(), &png[..length]).is_err());
    }
    assert!(Image::from_png(window.clone(), b"not png").is_err());
    let decoded = Image::from_png(window.clone(), &png).unwrap();
    png.fill(0);
    assert!(Image::from_png(window.clone(), &png).is_err());
    assert_eq!(decoded.size(), (8, 8));
    let first = Image::load(window.clone(), &fixture).unwrap();
    let second = Image::load(window.clone(), &fixture).unwrap();
    assert_eq!(first.size(), (8, 8));
    drop(first);
    let data = vec![255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 255];
    let image = Image::from_rgba(window.clone(), 2, 2, &data).unwrap();
    drop(data);
    let engine = Engine::new().unwrap();
    let resource = engine.resource(image).unwrap();
    let background = Color {
        r: 30,
        g: 40,
        b: 50,
        a: 255,
    };
    let frame = window.frame(background).unwrap();
    let region = Rectangle {
        x: 0,
        y: 0,
        width: 2,
        height: 2,
    };
    for bad in [
        Rectangle { x: -1, ..region },
        Rectangle { width: 3, ..region },
        Rectangle { width: 0, ..region },
    ] {
        assert!(
            frame
                .image_region(&resource, bad, Transform::default())
                .is_err()
        );
    }
    for bad in [
        Transform {
            scale: 0.0,
            ..Default::default()
        },
        Transform {
            position: (f32::NAN, 0.0),
            ..Default::default()
        },
        Transform {
            scale: f32::MAX,
            ..Default::default()
        },
    ] {
        assert!(frame.image_region(&resource, region, bad).is_err());
    }
    frame
        .image_region(
            &resource,
            region,
            Transform {
                position: (10.0, 10.0),
                scale: 10.0,
                ..Default::default()
            },
        )
        .unwrap();
    frame.image(&second, (80, 10)).unwrap();
    frame.image(&decoded, (80, 30)).unwrap();
    drop(decoded);
    pixel(
        81,
        31,
        Color {
            r: 255,
            g: 0,
            b: 0,
            a: 255,
        },
    );
    #[cfg(feature = "assets")]
    {
        let json = std::fs::read_to_string(fixture.with_file_name("atlas.json")).unwrap();
        let atlas = malvado::atlas::Atlas::parse(&json, 8, 8).unwrap();
        let wrong = Image::from_rgba(window.clone(), 1, 1, &[0; 4]).unwrap();
        assert!(matches!(
            malvado::atlas::TextureAtlas::new(
                malvado::atlas::Atlas::parse(&json, 8, 8).unwrap(),
                wrong
            ),
            Err(Error::InvalidArgument)
        ));
        let library =
            malvado::asset::Library::open(fixture.with_file_name("atlas.json"), None).unwrap();
        let textured = malvado::atlas::TextureAtlas::load(window.clone(), library).unwrap();
        assert_eq!(textured.image().size(), (8, 8));
        let temp =
            std::env::temp_dir().join(format!("malvado-texture-{}.json", std::process::id()));
        std::fs::write(&temp, json.replace("red.png", "missing-texture.png")).unwrap();
        let missing = malvado::asset::Library::open(&temp, None).unwrap();
        std::fs::remove_file(&temp).unwrap();
        assert!(malvado::atlas::TextureAtlas::load(window.clone(), missing).is_err());

        assert_eq!(
            textured.metadata().sprite(7).unwrap(),
            atlas.sprite(7).unwrap()
        );
        assert!(textured.sprite(-1).is_none());
        assert!(textured.sprite_named("RED").unwrap().is_none());
        assert!(matches!(
            textured.sprite_named("bad\0name"),
            Err(Error::InvalidArgument)
        ));
        let selected = textured.sprite_named("red").unwrap().unwrap();
        let clone = selected.clone();
        assert_eq!(selected.region(), atlas.sprite(7).unwrap().region);
        assert_eq!(selected.pivot(), atlas.sprite(7).unwrap().pivot);
        selected.shape().unwrap();
        drop(textured);
        drop(selected);
        assert!(frame.sprite(&clone, (120.0, 30.0), 0.0, 0.0).is_err());
        assert!(frame.sprite(&clone, (f32::NAN, 30.0), 1.0, 0.0).is_err());
        frame.sprite(&clone, (120.0, 30.0), 1.0, 0.0).unwrap();
        drop(clone);
        pixel(
            120,
            30,
            Color {
                r: 255,
                g: 0,
                b: 0,
                a: 255,
            },
        );
        let region = atlas.sprite(7).unwrap().region.into();
        frame
            .image_region(
                &second,
                region,
                Transform {
                    position: (100.0, 10.0),
                    ..Default::default()
                },
            )
            .unwrap();
        drop(atlas);
        pixel(
            101,
            11,
            Color {
                r: 255,
                g: 0,
                b: 0,
                a: 255,
            },
        );
    }
    #[cfg(feature = "assets")]
    {
        use std::io::Write;
        let bytes = std::fs::read(fixture.with_file_name("fpg").join("bennugd-8.fpg")).unwrap();
        let fpg = malvado::fpg::Fpg::parse(&bytes).unwrap();
        assert_eq!(fpg.pivot(7), Some((1, 0)));
        assert_eq!(fpg.pivot(42), Some((0, 0)));
        assert_eq!(fpg.pivot(-1), None);
        let textures = malvado::fpg::Textures::new(
            window.clone(),
            malvado::fpg::Fpg::parse(&bytes).unwrap(),
            true,
        )
        .unwrap();
        assert_eq!(textures.len(), 2);
        assert!(!textures.is_empty());
        assert_eq!(textures.at(1).unwrap().0, 42);
        assert!(textures.at(usize::MAX).is_none());
        assert!(textures.sprite(-1).is_none());
        assert!(matches!(
            textures.metadata(),
            malvado::asset::Metadata::Fpg(_)
        ));
        let opaque = malvado::fpg::Textures::new(
            window.clone(),
            malvado::fpg::Fpg::parse(&bytes).unwrap(),
            false,
        )
        .unwrap();
        frame
            .sprite(&opaque.sprite(7).unwrap(), (151.0, 50.0), 1.0, 0.0)
            .unwrap();
        drop(opaque);
        pixel(
            150,
            50,
            Color {
                r: 0,
                g: 0,
                b: 0,
                a: 255,
            },
        );
        let selected = textures.sprite(7).unwrap();
        assert_eq!(selected.pivot(), (1, 0));
        selected.shape().unwrap();
        drop(textures);
        frame.sprite(&selected, (131.0, 50.0), 1.0, 0.0).unwrap();
        drop(selected);
        pixel(
            131,
            50,
            Color {
                r: 252,
                g: 0,
                b: 0,
                a: 255,
            },
        );
        let dir =
            std::env::temp_dir().join(format!("malvado manifest textures {}", std::process::id()));
        std::fs::create_dir_all(&dir).unwrap();
        let path = dir.join("library.json");
        std::fs::write(&path, fpg.json(true).unwrap()).unwrap();
        for index in 0..fpg.len() {
            let id = fpg.at(index).unwrap().info.id;
            std::fs::write(dir.join(format!("{id}.png")), fpg.png(id, true).unwrap()).unwrap();
        }
        let library = malvado::asset::Library::open(&path, None).unwrap();
        let converted = malvado::fpg::Textures::load(window.clone(), library).unwrap();
        assert_eq!(converted.len(), 2);
        let malvado::asset::Metadata::Manifest(metadata) = converted.metadata() else {
            unreachable!()
        };
        assert_eq!(metadata.pivot(7), Some((1, 0)));
        assert_eq!(metadata.pivot(42), Some((0, 0)));
        assert_eq!(metadata.pivot(-1), None);
        let selected = converted.at(0).unwrap().1;
        drop(converted);
        // A partial load must release its first image when a later file fails.
        std::fs::remove_file(dir.join("42.png")).unwrap();
        let library = malvado::asset::Library::open(&path, None).unwrap();
        assert!(malvado::fpg::Textures::load(window.clone(), library).is_err());
        std::fs::copy(&fixture, dir.join("7.png")).unwrap();
        let library = malvado::asset::Library::open(&path, None).unwrap();
        // The selected sprite still retains its cached image. Release it before
        // checking a replacement file at the same native cache path.
        frame.sprite(&selected, (141.0, 50.0), 1.0, 0.0).unwrap();
        drop(selected);
        // A distinct path avoids intentionally retained frame cache references.
        let wrong = dir.join("wrong");
        std::fs::create_dir_all(&wrong).unwrap();
        std::fs::copy(&fixture, wrong.join("7.png")).unwrap();
        std::fs::write(wrong.join("library.json"), fpg.json(true).unwrap()).unwrap();
        let bad = malvado::asset::Library::open(wrong.join("library.json"), None).unwrap();
        assert!(matches!(
            malvado::fpg::Textures::load(window.clone(), bad),
            Err(Error::InvalidArgument)
        ));
        drop(library);
        std::fs::remove_dir_all(dir).unwrap();
        pixel(
            141,
            50,
            Color {
                r: 252,
                g: 0,
                b: 0,
                a: 255,
            },
        );
        let image = fpg.image(window.clone(), 7, true).unwrap();
        let png = fpg.png(7, true).unwrap();
        let nonce = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_nanos();
        let path =
            std::env::temp_dir().join(format!("malvado-fpg-{}-{nonce}.png", std::process::id()));
        let mut file = std::fs::OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(&path)
            .unwrap();
        file.write_all(&png).unwrap();
        drop(file);
        let exported = Image::load(window.clone(), &path).unwrap();
        std::fs::remove_file(path).unwrap();
        drop(fpg); // Both GPU images are independent of the source library.
        let region = Rectangle {
            x: 0,
            y: 0,
            width: 2,
            height: 2,
        };
        frame
            .image_region(
                &image,
                region,
                Transform {
                    position: (120.0, 10.0),
                    scale: 10.0,
                    ..Default::default()
                },
            )
            .unwrap();
        frame
            .image_region(
                &exported,
                region,
                Transform {
                    position: (120.0, 50.0),
                    scale: 10.0,
                    ..Default::default()
                },
            )
            .unwrap();
        drop(image);
        drop(exported);
        pixel(125, 15, background);
        pixel(
            135,
            15,
            Color {
                r: 252,
                g: 0,
                b: 0,
                a: 255,
            },
        );
        pixel(125, 55, background);
        pixel(
            135,
            55,
            Color {
                r: 252,
                g: 0,
                b: 0,
                a: 255,
            },
        );
    }
    // Queued references survive both resource/context cleanup and caller drops.
    drop(resource);
    drop(engine);
    drop(second);
    pixel(
        15,
        15,
        Color {
            r: 255,
            g: 0,
            b: 0,
            a: 255,
        },
    );
    pixel(25, 15, background);
    pixel(
        15,
        25,
        Color {
            r: 0,
            g: 255,
            b: 0,
            a: 255,
        },
    );
    pixel(
        25,
        25,
        Color {
            r: 0,
            g: 0,
            b: 255,
            a: 255,
        },
    );
    pixel(
        81,
        11,
        Color {
            r: 255,
            g: 0,
            b: 0,
            a: 255,
        },
    );
    drop(frame);
    assert_eq!(Rc::strong_count(&window), 1);
    let pending = Image::from_rgba(window.clone(), 1, 1, &[255, 0, 0, 255]).unwrap();
    let slot = Rc::new(RefCell::new(None));
    let process_slot = slot.clone();
    let mut engine = Engine::new().unwrap();
    let handle = engine
        .spawn(Default::default(), move |process| async move {
            *process_slot.borrow_mut() = Some(process.resource(pending)?);
            process.frame().await;
            Ok(())
        })
        .unwrap();
    engine.tick().unwrap();
    let resource = slot.borrow_mut().take().unwrap();
    let frame = window.frame(background).unwrap();
    frame
        .image_region(
            &resource,
            Rectangle {
                x: 0,
                y: 0,
                width: 1,
                height: 1,
            },
            Transform {
                position: (10.0, 50.0),
                scale: 20.0,
                ..Default::default()
            },
        )
        .unwrap();
    drop(resource);
    assert!(handle.kill().unwrap());
    assert_eq!(engine.process_count(), 0);
    pixel(
        15,
        55,
        Color {
            r: 255,
            g: 0,
            b: 0,
            a: 255,
        },
    );
    drop(frame);
    drop(engine);
    assert_eq!(Rc::strong_count(&window), 1);
    let image = Image::from_rgba(window.clone(), 1, 1, &[255, 0, 0, 255]).unwrap();
    let cloned = image.clone();
    drop(image);
    drop(window);
    assert!(weak.upgrade().is_some());
    assert!(matches!(
        Window::open(32, 32, "retained window"),
        Err(Error::Busy)
    ));
    assert_eq!(cloned.size(), (1, 1));
    drop(cloned);
    assert!(weak.upgrade().is_none());
    drop(Window::open(32, 32, "after final image drop").unwrap());
}
