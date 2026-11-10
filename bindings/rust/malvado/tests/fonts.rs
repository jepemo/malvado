// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Engine, Error,
    font::Font,
    window::{Color, Window},
};
use std::{cell::RefCell, path::Path, rc::Rc};
unsafe extern "C" {
    fn malvado_test_frame_hash(light: *mut i32) -> u64;
}
fn capture() -> u64 {
    let mut light = 0;
    // SAFETY: live main-thread frame and valid output; helper retains nothing.
    let hash = unsafe { malvado_test_frame_hash(&mut light) };
    assert!(light > 10);
    hash
}
fn main() {
    let window = Rc::new(Window::open(200, 100, "Rust fonts").unwrap());
    let weak = Rc::downgrade(&window);
    let path = Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../../../tests/fixtures/fonts/AnonymousPro-Bold.ttf");
    assert!(Font::load(window.clone(), path.with_file_name("missing.ttf"), 24).is_err());
    assert!(matches!(
        Font::load(window.clone(), &path, 0),
        Err(Error::InvalidArgument)
    ));
    assert!(matches!(
        Font::load(window.clone(), "bad\0path", 24),
        Err(Error::InvalidArgument)
    ));
    for glyphs in [&[][..], &['\0'][..], &['A', 'A'][..]] {
        assert!(matches!(
            Font::load_glyphs(window.clone(), &path, 24, glyphs),
            Err(Error::InvalidArgument)
        ));
    }
    assert!(Font::load_glyphs(window.clone(), &path, 24, &vec!['A'; 1025]).is_err());
    let basic = Font::load(window.clone(), &path, 24).unwrap();
    let otf = Font::load(
        window.clone(),
        path.with_file_name("SourceCodePro-Regular.otf"),
        24,
    )
    .unwrap();
    assert!(otf.text_width("OpenType", 24).unwrap() > 0);
    drop(otf);
    assert!(basic.text_width("Café", 24).unwrap() > 0);
    assert_eq!(basic.text_width("", 24).unwrap(), 0);
    assert!(basic.text_width("bad\0text", 24).is_err());
    assert!(basic.text_width("text", 1025).is_err());
    let black = Color {
        r: 0,
        g: 0,
        b: 0,
        a: 255,
    };
    let white = Color {
        r: 255,
        g: 255,
        b: 255,
        a: 255,
    };
    let frame = window.frame(black).unwrap();
    assert!(
        frame
            .font_text(&basic, "bad\0text", (10, 10), 24, white)
            .is_err()
    );
    assert!(frame.font_text(&basic, "text", (10, 10), 0, white).is_err());
    frame.font_text(&basic, "Ω", (20, 20), 24, white).unwrap();
    let fallback = capture();
    drop(frame);
    drop(basic);
    let glyphs = ['?', 'Ω', 'C', 'a', 'f', 'é', ' '];
    let selected = Font::load_glyphs(window.clone(), &path, 24, &glyphs).unwrap();
    let mut engine = Engine::new().unwrap();
    let slot = Rc::new(RefCell::new(None));
    let owned = slot.clone();
    let handle = engine
        .spawn(Default::default(), move |process| async move {
            *owned.borrow_mut() = Some(process.resource(selected)?);
            process.frame().await;
            Ok(())
        })
        .unwrap();
    engine.tick().unwrap();
    let resource = slot.borrow_mut().take().unwrap();
    let frame = window.frame(black).unwrap();
    frame
        .font_text(&resource, "Ω", (20, 20), 24, white)
        .unwrap();
    drop(resource);
    assert!(handle.kill().unwrap());
    drop(engine);
    assert_ne!(capture(), fallback); // Selected Unicode glyph differs from fallback.
    drop(frame);
    assert_eq!(Rc::strong_count(&window), 1);
    let font = Font::load(window.clone(), &path, 24).unwrap();
    let retained = font.clone();
    drop(font);
    drop(window);
    assert!(weak.upgrade().is_some());
    assert!(retained.text_width("Café", 24).unwrap() > 0);
    drop(retained);
    assert!(weak.upgrade().is_none());
    drop(Window::open(32, 32, "After font cleanup").unwrap());
}
