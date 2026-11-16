// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error,
    asset::{Library, Metadata, Source},
    image::Image,
    window::Window,
};
use std::rc::Rc;
pub fn load(window: Rc<Window>) -> (Source, Result<Image, Error>) {
    // Use an absolute sandbox path: raylib 6.2 resolves relative reads from
    // packaged assets first. Unlink must remove the actual snapshot source.
    let snapshot = Source::open(std::env::current_dir().unwrap().join("message.bin")).unwrap();
    assert_eq!(snapshot.bytes(), &[0, 13, 10, 255]);
    std::fs::remove_file("message.bin").unwrap();
    assert_eq!(snapshot.bytes(), &[0, 13, 10, 255]);
    let Metadata::Atlas(atlas) = Library::open("atlas.json", None).unwrap().into_metadata() else {
        panic!("expected atlas")
    };
    assert_eq!(atlas.sprite(42).unwrap().pivot, (2, 4));
    let image = Image::load(window.clone(), "red.png");
    if image.is_err() {
        return (snapshot, image);
    }
    assert_eq!(image.as_ref().unwrap().size(), (8, 8));
    let decoded = Image::from_png(window, &std::fs::read("red.png").unwrap()).unwrap();
    assert_eq!(decoded.size(), (8, 8));
    drop(decoded);
    (snapshot, image)
}
